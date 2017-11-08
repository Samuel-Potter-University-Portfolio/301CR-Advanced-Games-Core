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
	ByteBuffer& buffer = socketType == TCP ? m_TcpCallQueue : m_UdpCallQueue;

	// Encode function call information
	Encode<uint16>(buffer, id);
	Encode<uint8>(buffer, (uint8)target);
	Encode<uint16>(buffer, params.Size());
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