#include "Includes\Core\NetSerializableBase.h"
#include "Includes\Core\NetSession.h"
#include <cstring>


void NetSerializableBase::UpdateRole(const NetSession* session, const bool& assignOwner)
{
	if (assignOwner)
		m_networkOwnerId = session != nullptr ? session->GetSessionNetID() : 0;

	// Doesn't sync, so don't care
	if (!IsNetSynced())
		m_netRole = NetRole::None;

	// Is local playing
	else if (session == nullptr)
		m_netRole = NetRole::HostOwner;

	// Hosting server
	else if (session->IsHost())
		m_netRole = m_networkOwnerId == session->GetSessionNetID() ? NetRole::HostOwner : NetRole::HostPuppet;

	// Connected to server
	else
		m_netRole = m_networkOwnerId == session->GetSessionNetID() ? NetRole::RemoteOwner : NetRole::RemotePuppet;
}

void NetSerializableBase::OnPreNetUpdate() 
{
	// Fetch all variables that need to be updated
	if (IsNetHost() && GetNetworkID() != 0)
	{
		uint16 index;
		uint32 track;
		m_updateCounter++;
		// If first update, encode all variables over TCP
		RegisterSyncVars(m_TcpVarQueue, TCP, index, track, false); 
		RegisterSyncVars(m_UdpVarQueue, UDP, index, track, false);
	}
}



bool NetSerializableBase::RegisterRPCs(const char* func, RPCInfo& outInfo) const 
{
	outInfo.index = 0;
	return false;
}
bool NetSerializableBase::ExecuteRPC(uint16& id, ByteBuffer& params) 
{
	return false;
}


void NetSerializableBase::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode)
{
	index = 0;
	trackIndex = 0;
}
bool NetSerializableBase::ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks)
{
	return false;
}


void NetSerializableBase::RemoteCallRPC(const RPCInfo& rpcInfo, const ByteBuffer& params) 
{
	if (!IsNetSynced())
	{
		LOG_ERROR("Cannot call RPCs for a non-net synced class");
		return;
	}
	
	// Insert into appropriate queue
	RPCQueue& queue = (rpcInfo.socket == SocketType::TCP ? m_TcpRpcQueue : m_UdpRpcQueue);
	RPCRequest request;
	request.function = rpcInfo;
	request.params = params;
	queue.emplace_back(request);
}


void NetSerializableBase::EncodeRPCRequests(const uint16& targetNetId, ByteBuffer& buffer, const SocketType& socketType)
{
	// Doesn't have control, so shouldn't even be here
	if (!HasNetControl())
		return;

	RPCQueue& queue = (socketType == SocketType::TCP ? m_TcpRpcQueue : m_UdpRpcQueue);
	uint16 callCount = 0;
	ByteBuffer callBuffer;


	// Decide which calls need to be encoded
	for (const RPCRequest& request : queue)		
	{
		// Only encode relevent calls
		if (
			// Client, so just queue request
			!IsNetHost() ||

			// Server, so send to correct clients
			(request.function.callingMode == RPCCallingMode::Owner && targetNetId == m_networkOwnerId) ||
			(request.function.callingMode == RPCCallingMode::Broadcast)
		)
		{
			Encode<RPCRequest>(callBuffer, request);
			++callCount;
		}
	}
	

	// Encode calls into main buffer
	Encode<uint16>(buffer, callCount);
	if(callCount != 0)
		buffer.Push(callBuffer.Data(), callBuffer.Size());
}

void NetSerializableBase::DecodeRPCRequests(const uint16& sourceNetId, ByteBuffer& buffer, const SocketType& socketType)
{
	uint16 count;
	if (!Decode(buffer, count) || count == 0)
		return;


	// Doesn't have permission to call RPC, so get rid of excess and ignore
	const bool skipExecution = (sourceNetId != 0 && sourceNetId != m_networkOwnerId && !IsNetHost());
	RPCQueue& queue = (socketType == SocketType::TCP ? m_TcpRpcQueue : m_UdpRpcQueue);


	// Execute all RPCs or add them to output (If server) for relaying
	for (uint32 i = 0; i < count; ++i)
	{
		RPCRequest request;
		request.function.socket = socketType;
		if (!Decode<RPCRequest>(buffer, request))
			return;
		if (skipExecution)
			continue;

		if (IsNetHost())
		{
			if (request.function.callingMode == RPCCallingMode::Host)
				ExecuteRPC(request.function.index, request.params);

			else if (request.function.callingMode == RPCCallingMode::Broadcast)
				queue.emplace_back(request);
		}
		else 
		{
			if(request.function.callingMode != RPCCallingMode::Host)
				ExecuteRPC(request.function.index, request.params);
		}
	}
}


void NetSerializableBase::EncodeSyncVarRequests(const uint16& targetNetId, ByteBuffer& buffer, const SocketType& socketType, const bool& forceEncode)
{
	// Only host can sync vars
	if (!IsNetHost())
	{
		Encode<uint16>(buffer, 0);
		return;
	}

	SyncVarQueue* queue;
	SyncVarQueue tempQueue; // Keep it in scope
	if (forceEncode)
	{
		uint16 index;
		uint32 track;
		RegisterSyncVars(tempQueue, socketType, index, track, forceEncode);
		queue = &tempQueue;
	}
	else
		queue = &(socketType == SocketType::TCP ? m_TcpVarQueue : m_UdpVarQueue);

	uint16 count = 0;
	ByteBuffer callBuffer;

	// Encode all var changes (Synced to every client)
	for (const SyncVarRequest& request : *queue)
	{
		Encode<SyncVarRequest>(callBuffer, request);
		++count;
	}

	// Encode calls into main buffer
	Encode<uint16>(buffer, count);
	if (count != 0)
		buffer.Push(callBuffer.Data(), callBuffer.Size());
}

void NetSerializableBase::DecodeSyncVarRequests(const uint16& sourceNetId, ByteBuffer& buffer, const SocketType& socketType, const bool& skipCallbacks)
{
	uint16 count;
	if (!Decode(buffer, count) || count == 0)
		return;

	// Only process var changes if they have come from the server
	const bool skipExecution = sourceNetId != 0 || IsNetHost();

	// Update all sync vars
	for (uint32 i = 0; i < count; ++i)
	{
		SyncVarRequest request;
		request.variable.socket = socketType;
		if (!Decode<SyncVarRequest>(buffer, request))
			return;
		if (skipExecution)
			continue;

		ExecuteSyncVar(request.variable.index, request.value, skipCallbacks);
	}
}