#include "Includes\Core\NetSerializableBase.h"
#include <cstring>


void NetSerializableBase::RemoteCallRPC(const uint16& id, const ByteBuffer& params, const RPCTarget& target, const SocketType& socketType)
{
	if (!bNetSynced)
	{
		LOG_ERROR("Cannot call RPCs for a non-net synced class");
		return;
	}

	// Select appropriate queue
	ByteBuffer& buffer = (socketType == TCP ? m_TcpCallQueue : m_UdpCallQueue);

	// Encode function call information
	Encode<uint16>(buffer, id);
	Encode<uint8>(buffer, (uint8)target);
	if (params.Size() != 0)
		buffer.Push(params.Data(), params.Size());

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

void NetSerializableBase::PerformNetEncode(ByteBuffer& buffer, const SocketType& socketType) 
{
	// Encode RPC calls
	ByteBuffer& rpcBuffer = (socketType == SocketType::TCP ? m_TcpCallQueue : m_UdpCallQueue);
	Encode<uint16>(buffer, rpcBuffer.Size());
	buffer.Push(rpcBuffer.Data(), rpcBuffer.Size());
	rpcBuffer.Clear();
}

void NetSerializableBase::PerformNetDecode(ByteBuffer& buffer, const SocketType& socketType)
{
	// Decode and execute RPC calls
	uint16 rpcSize;
	Decode<uint16>(buffer, rpcSize);
	uint32 endSize = buffer.Size() - rpcSize;

	// Try to decode and execute all RPCs
	while (buffer.Size() > endSize)
	{
		uint16 index;
		uint8 rawTarget;
		Decode(buffer, index);
		Decode(buffer, rawTarget);

		RPCTarget target = (RPCTarget)rawTarget;
		if (!ExecuteRPC(index, buffer))
		{
			LOG_WARNING("Received bad RPC request");
		}
	}
}