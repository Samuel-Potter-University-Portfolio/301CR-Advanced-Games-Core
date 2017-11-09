#include "Includes\Core\NetSerializableBase.h"
#include <cstring>


void NetSerializableBase::RemoteCallRPC(const uint16& id, const ByteBuffer& params, const RPCTarget& target, const SocketType& socketType)
{
	if (!bNetSynced)
	{
		LOG_ERROR("Cannot call RPCs for a non-net synced class");
		return;
	}
	
	// TODO - Check target is valid


	// Insert into appropriate queue
	RPCQueue& queue = (socketType == SocketType::TCP ? m_TcpRpcQueue : m_UdpRpcQueue);
	RPCRequest request;
	request.index = id;
	request.target = target;
	request.params.Push(params.Data(), params.Size());
	queue.emplace_back(request);


	// TODO - Move into macro call, as this is really inefficient (It will also help validate params, as macro will not compile otherwise)
	if (target == RPCTarget::GlobalBroadcast)
	{
		uint16 tempId = id;
		ByteBuffer tempBuffer(params);
		tempBuffer.Flip();
		ExecuteRPC(tempId, tempBuffer);
	}

	// TODO - Checks to see if need to be executed here? Is host?
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

void NetSerializableBase::EncodeRPCRequests(ByteBuffer& buffer, const SocketType& socketType)
{
	RPCQueue& queue = (socketType == SocketType::TCP ? m_TcpRpcQueue : m_UdpRpcQueue);

	Encode<uint16>(buffer, queue.size()); // Encode number of calls
	for (const RPCRequest& request : queue) // Encode each call
		Encode<RPCRequest>(buffer, request);
}

void NetSerializableBase::DecodeRPCRequests(ByteBuffer& buffer, RPCQueue& output)
{
	uint16 count;
	if (!Decode(buffer, count))
		return;

	for (uint32 i = 0; i < count; ++i)
	{
		RPCRequest call;
		if (!Decode(buffer, call))
			continue;
		output.emplace_back(call);
	}
}

void NetSerializableBase::ClearQueuedNetData()
{
	m_UdpRpcQueue.clear();
	m_TcpRpcQueue.clear();
}