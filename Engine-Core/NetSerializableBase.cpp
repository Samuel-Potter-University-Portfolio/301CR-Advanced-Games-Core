#include "Includes\Core\NetSerializableBase.h"
#include "Includes\Core\NetSession.h"
#include <cstring>


void NetSerializableBase::UpdateRole(const NetSession* session, const bool& assignOwner)
{
	if (assignOwner)
		m_networkOwnerId = session != nullptr ? session->GetNetworkID() : 0;

	// Doesn't sync, so don't care
	if (!IsNetSynced())
		m_netRole = NetRole::None;

	// Is local playing
	else if (session == nullptr)
		m_netRole = NetRole::HostOwner;

	// Hosting server
	else if (session->IsHost())
		m_netRole = m_networkOwnerId == session->GetNetworkID() ? NetRole::HostOwner : NetRole::HostPuppet;

	// Connected to server
	else
		m_netRole = m_networkOwnerId == session->GetNetworkID() ? NetRole::RemoteOwner : NetRole::RemotePuppet;
}

void NetSerializableBase::RemoteCallRPC(const uint16& id, const ByteBuffer& params, const RPCTarget& target, const SocketType& socketType)
{
	if (!IsNetSynced())
	{
		LOG_ERROR("Cannot call RPCs for a non-net synced class");
		return;
	}
	
	// Insert into appropriate queue
	RPCQueue& queue = (socketType == SocketType::TCP ? m_TcpRpcQueue : m_UdpRpcQueue);
	RPCRequest request;
	request.index = id;
	request.target = target;
	request.params.Push(params.Data(), params.Size());
	queue.emplace_back(request);
}

bool NetSerializableBase::FetchRPCIndex(const char* funcName, uint16& outID) const
{
	outID = 0;
	return false;
}

bool NetSerializableBase::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	return false;
}

void NetSerializableBase::EncodeRPCRequests(const uint16& targetNetId, ByteBuffer& buffer, const SocketType& socketType)
{
	// Doesn't have control, so shouldn't even be here
	if (!HasNetControl())
		return;

	RPCQueue& queue = (socketType == SocketType::TCP ? m_TcpRpcQueue : m_UdpRpcQueue);
	uint16 callCount = 0;
	ByteBuffer tempBuffer;

	// Decide which calls need to be encoded
	for (const RPCRequest& request : queue)		
	{
		// Server, so send to correct clients
		if (IsNetHost())
		{
			if (request.target == RPCTarget::ClientBroadcast || request.target == RPCTarget::GlobalBroadcast || (targetNetId == m_networkOwnerId && request.target == RPCTarget::Owner))
			{
				Encode<RPCRequest>(tempBuffer, request);
				++callCount;
			}
		}

		// Client, so only allow valid calls to go through to the server
		else if (request.target == RPCTarget::ClientBroadcast || request.target == RPCTarget::GlobalBroadcast || request.target == RPCTarget::Host)
		{
			Encode<RPCRequest>(tempBuffer, request);
			++callCount;
		}
	}

	Encode<uint16>(buffer, callCount);
	if(callCount != 0)
		buffer.Push(tempBuffer.Data(), tempBuffer.Size());
}

void NetSerializableBase::DecodeRPCRequests(const uint16& sourceNetId, ByteBuffer& buffer, const SocketType& socketType)
{
	uint16 count;
	if (!Decode(buffer, count) || count == 0)
		return;


	// Doesn't have permission to call RPC, so get rid of excess and ignore
	if (sourceNetId != 0 && sourceNetId != m_networkOwnerId && !IsNetHost())
	{
		ByteBuffer bin;
		buffer.PopBuffer(bin, count);
		return;
	}

	RPCQueue& queue = (socketType == SocketType::TCP ? m_TcpRpcQueue : m_UdpRpcQueue);


	// Execute all RPCs or re-en
	for (uint32 i = 0; i < count; ++i)
	{
		RPCRequest call;
		if (!Decode(buffer, call))
			continue;


		// Encode or push to outgoing queue
		if (IsNetHost())
		{
			// Execute function on host, if desired
			if (call.target == RPCTarget::Host || call.target == RPCTarget::GlobalBroadcast || (IsNetOwner() && call.target == RPCTarget::Owner))
				ExecuteRPC(call.index, call.params);

			// Broadcast to other clients
			if (call.target == RPCTarget::GlobalBroadcast || call.target == RPCTarget::ClientBroadcast || (!IsNetOwner() && call.target == RPCTarget::Owner))
				queue.emplace_back(call);
		}

		// Execute the call, as it's come from the host, so must be meant for this client
		else
			ExecuteRPC(call.index, call.params);
	}
}