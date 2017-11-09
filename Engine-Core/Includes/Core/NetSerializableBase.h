/**
* Used macros to provide a nice API approach to adding support of RPCs and synced vars
*/
#pragma once
#include "Common.h"
#include "Encoding.h"
#include "ByteBuffer.h"
#include "NetSocket.h"


/**
* All the different possible ways to execute RPCs
*/
enum class RPCTarget : uint8
{
	Unknown = 0,
	Owner = 1,				// Call to be executed on the client of whoever owns this object
	Host = 2,				// Call to be executed on the host of this current session
	ClientBroadcast = 3,	// Call to be executed on all clients, but not the server (Will only executed on server, if host is also a client)
	GlobalBroadcast = 4,	// Call to be executed on all clients and server (Will only execute once on host, if they are a client)

	This,					// Call should be executed on this client ASAP (Only to be used by server, when forcing execution on a client)
};


/**
* All the different roles a net synced object can have
*/
enum class NetRole : uint8 
{
	None = 0,
	RemotePuppet,			// Doesn't have any control over this object whatsoever
	HostPuppet,				// Doesn't own this object, but is the host, so can edit
	RemoteOwner,			// Owns the object, but is not the host
	HostOwner,				// Owns the object and is the host
};

/**
* Contains all information about a RPC call
*/
struct RPCRequest
{
	uint16		index;
	RPCTarget	target;
	ByteBuffer	params;
};
typedef std::vector<RPCRequest> RPCQueue;

template<>
inline void Encode<RPCRequest>(ByteBuffer& buffer, const RPCRequest& data)
{
	Encode<uint16>(buffer, data.index);
	Encode<uint8>(buffer, (uint8)data.target);
	Encode<uint16>(buffer, data.params.Size());
	buffer.Push(data.params.Data(), data.params.Size());
}

template<>
inline bool Decode<RPCRequest>(ByteBuffer& buffer, RPCRequest& out, void* context)
{
	uint8 target;
	uint16 paramCount;

	if (!Decode<uint16>(buffer, out.index) ||
		!Decode<uint8>(buffer, target) ||
		!Decode<uint16>(buffer, paramCount) 
		) 
		return false;

	out.target = (RPCTarget)target;
	buffer.PopBuffer(out.params, paramCount);
	return true;
}


/**
* Lets child classes register RPCs and synced variables
* O-------------------------O
* RPCs (For best performance they shouldn't return anything):
* Override both FetchRPCIndex and ExecuteRPC.
* -FetchRPCIndex: place RPC_INDEX_HEADER at top passing passed arguments and use RPC_INDEX for each function you want to register
* -ExecuteRPC: place RPC_EXEC_HEADER at top passing passed arguments and use RPC_EXEC for each function you want to register (In the same order as in RPC_INDEX)
* --If you want to call a function that has arguments use RPC_EXEC_OneParam, RPC_EXEC_TwoParam etc. up until Five
* --If you want more that 5 arguments use a struct and add an Encode and Decode template implementation
* O-------------------------O
*/
class CORE_API NetSerializableBase
{
private:
	friend class NetSession;
	friend class Level;
	NetRole m_netRole = NetRole::None;
	uint16 m_networkOwnerId = 0;
	uint16 m_networkId = 0;

	RPCQueue m_UdpRpcQueue;
	RPCQueue m_TcpRpcQueue;

protected:
	bool bNetSynced = false;

public:
	/**
	* Fetch the function's ID from the RPC table
	* NOTE: macro order between FetchRPCIndex and ExecuteRPC must align
	* @param funcName			The name of the function
	* @param outID				The functions index in the RPC table
	* @returns If the function is registered in the table
	*/
	virtual bool FetchRPCIndex(const char* funcName, uint16& outID) const;

	/**
	* Enqueue an RPC to be executed by the server, client
	* (If no session is active, they will just execute normally)
	* @param id				The function's RPC index
	* @param params			Encoded parameters to call the function using
	* @param target			The target mode of calling to use for this RPC
	* @param socketType		The socket type for this call to be made over
	*/
	void RemoteCallRPC(const uint16& id, const ByteBuffer& params, const RPCTarget& target, const SocketType& socketType);

protected:
	/**
	* Call a given function fromt it's RPC id
	* NOTE: macro order between FetchRPCIndex and ExecuteRPC must align
	* @param id				The RPC id of the function
	* @param params			The raw parameters for the function to use
	* @returns If call succeeds
	*/
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params);

	/**
	* Clears any net data which is currently queued
	*/
	void ClearQueuedNetData();


private:
	/**
	* Encode all currently queued RPC calls
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this will be sent over
	*/
	void EncodeRPCRequests(ByteBuffer& buffer, const SocketType& socketType);
	/**
	* Decode all RPC calls in this queue
	* @param buffer			The buffer to fill with all this information
	* @param output			Where to store all calls
	*/
	void DecodeRPCRequests(ByteBuffer& buffer, RPCQueue& output);


protected:
	/**
	* Any additional information to be encoded for this object
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this will be sent over
	*/
	virtual void EncodeExtra(ByteBuffer& buffer, const SocketType& socketType) {}
	/**
	* Any additional information that needs to be decoded for this object (In the same format as encoding)
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this was recieved by
	*/
	virtual void DecodeExtra(ByteBuffer& buffer, const SocketType& socketType) {}


	/**
	* Getters & Setters
	*/
public:
	inline const bool& IsNetSynced() const { return bNetSynced; }

	/**
	* A unique ID for identifying this object over the current NetSession
	*/
	inline const uint16& GetNetworkID() const { return m_networkId; }

	inline const NetRole& GetNetRole() const { return m_netRole; }
	inline const bool IsNetOwner() const { return m_netRole == NetRole::HostOwner || m_netRole == NetRole::RemoteOwner; }
	inline const bool IsNetHost() const { return m_netRole == NetRole::HostOwner || m_netRole == NetRole::HostPuppet; }
	inline const bool HasNetControl() const { return IsNetOwner() || IsNetHost(); }
};



/**
* Placed at the start of FetchRPCIndex to create temporary vars (To avoid naming problems)
* and to handle parent calls correctly
*/
#define RPC_INDEX_HEADER(func, id) \
	const char*& __TEMP_NAME = func; \
	uint16& __TEMP_ID = outID; \
	if(__super::FetchRPCIndex(__TEMP_NAME, __TEMP_ID)) return true; 

/**
* Placed after RPC_INDEX_HEADER in FetchRPCIndex to create an entry for a function
*/
#define RPC_INDEX(func) \
	if (std::strcmp(__TEMP_NAME, #func) == 0) \
		return true; \
	else \
		++__TEMP_ID;




/**
* Placed at the start of ExecuteRPC to create temporary vars (To avoid naming problems)
* and to handle parent calls correctly
*/
#define RPC_EXEC_HEADER(id, params) \
	uint16 __TEMP_ID = id; \
	ByteBuffer& __TEMP_BUFFER = params; \
	if(__super::ExecuteRPC(__TEMP_ID, __TEMP_BUFFER)) return true;


/**
* Execution for a function at the placed index
* Containing 0 parameters
*/
#define RPC_EXEC(func) \
	if (__TEMP_ID == 0) \
	{ \
		func(); \
		return true; \
	} \
	else \
		--__TEMP_ID;

/**
* Execution for a function at the placed index
* Containing 1 parameters
*/
#define RPC_EXEC_OneParam(func, paramAType) \
	if (__TEMP_ID == 0) \
	{ \
		paramAType A; \
		if(Decode<paramAType>(__TEMP_BUFFER, A)) \
			func(A); \
		return true; \
	} \
	else \
		--__TEMP_ID;

/**
* Execution for a function at the placed index
* Containing 2 parameters
*/
#define RPC_EXEC_TwoParam(func, paramAType, paramBType) \
	if (__TEMP_ID == 0) \
	{ \
		paramAType A; \
		paramBType B; \
		if(	Decode<paramAType>(__TEMP_BUFFER, A) && \
			Decode<paramBType>(__TEMP_BUFFER, B)) \
			func(A, B); \
		return true; \
	} \
	else \
		--__TEMP_ID;

/**
* Execution for a function at the placed index
* Containing 3 parameters
*/
#define RPC_EXEC_ThreeParam(func, paramAType, paramBType, paramCType) \
	if (__TEMP_ID == 0) \
	{ \
		paramAType A; \
		paramBType B; \
		paramCType C; \
		if(	Decode<paramAType>(__TEMP_BUFFER, A) && \
			Decode<paramBType>(__TEMP_BUFFER, B) && \
			Decode<paramCType>(__TEMP_BUFFER, C)) \
			func(A, B, C); \
		return true; \
	} \
	else \
		--__TEMP_ID;

/**
* Execution for a function at the placed index
* Containing 4 parameters
*/
#define RPC_EXEC_FourParam(func, paramAType, paramBType, paramCType, paramDType) \
	if (__TEMP_ID == 0) \
	{ \
		paramAType A; \
		paramBType B; \
		paramCType C; \
		paramDType D; \
		if(	Decode<paramAType>(__TEMP_BUFFER, A) && \
			Decode<paramBType>(__TEMP_BUFFER, B) && \
			Decode<paramCType>(__TEMP_BUFFER, C) && \
			Decode<paramDType>(__TEMP_BUFFER, D)) \
			func(A, B, C, D); \
		return true; \
	} \
	else \
		--__TEMP_ID;

/**
* Execution for a function at the placed index
* Containing 4 parameters
*/
#define RPC_EXEC_FiveParam(func, paramAType, paramBType, paramCType, paramDType, paramEType) \
	if (__TEMP_ID == 0) \
	{ \
		paramAType A; \
		paramBType B; \
		paramCType C; \
		paramDType D; \
		paramEType E; \
		if(	Decode<paramAType>(__TEMP_BUFFER, A) && \
			Decode<paramBType>(__TEMP_BUFFER, B) && \
			Decode<paramCType>(__TEMP_BUFFER, C) && \
			Decode<paramDType>(__TEMP_BUFFER, D) && \
			Decode<paramEType>(__TEMP_BUFFER, E)) \
			func(A, B, C, D, E); \
		return true; \
	} \
	else \
		--__TEMP_ID;




/**
* Execute RPC with given settings
* Containing 0 parameters
*/
#define CallRPC(socket, mode, object, func) {\
	NetSerializableBase* __TEMP_NSB = (NetSerializableBase*)object; \
	uint16 __TEMP_ID; \
	if(__TEMP_NSB->FetchRPCIndex(#func, __TEMP_ID)) \
	{ \
		ByteBuffer __TEMP_BUFFER; \
		__TEMP_NSB->RemoteCallRPC(__TEMP_ID, __TEMP_BUFFER, mode, socket); \
	} \
	else \
		LOG_ERROR("Cannot call function '" #func "' as it is not a registered RPC for the given object"); \
}

/**
* Execute RPC with given settings
* Containing 1 parameters
*/
#define CallRPC_OneParam(socket, mode, object, func, paramA) {\
	NetSerializableBase* __TEMP_NSB = (NetSerializableBase*)object; \
	uint16 __TEMP_ID; \
	if(__TEMP_NSB->FetchRPCIndex(#func, __TEMP_ID)) \
	{ \
		ByteBuffer __TEMP_BUFFER; \
		Encode(__TEMP_BUFFER, paramA);\
		__TEMP_NSB->RemoteCallRPC(__TEMP_ID, __TEMP_BUFFER, mode, socket); \
	} \
	else \
		LOG_ERROR("Cannot call function '" #func "' as it is not a registered RPC for the given object"); \
}

/**
* Execute RPC with given settings
* Containing 2 parameters
*/
#define CallRPC_TwoParam(socket, mode, object, func, paramA, paramB) {\
	NetSerializableBase* __TEMP_NSB = (NetSerializableBase*)object; \
	uint16 __TEMP_ID; \
	if(__TEMP_NSB->FetchRPCIndex(#func, __TEMP_ID)) \
	{ \
		ByteBuffer __TEMP_BUFFER; \
		Encode(__TEMP_BUFFER, paramA);\
		Encode(__TEMP_BUFFER, paramB);\
		__TEMP_NSB->RemoteCallRPC(__TEMP_ID, __TEMP_BUFFER, mode, socket); \
	} \
	else \
		LOG_ERROR("Cannot call function '" #func "' as it is not a registered RPC for the given object"); \
}

/**
* Execute RPC with given settings
* Containing 3 parameters
*/
#define CallRPC_ThreeParam(socket, mode, object, func, paramA, paramB, paramC) {\
	NetSerializableBase* __TEMP_NSB = (NetSerializableBase*)object; \
	uint16 __TEMP_ID; \
	if(__TEMP_NSB->FetchRPCIndex(#func, __TEMP_ID)) \
	{ \
		ByteBuffer __TEMP_BUFFER; \
		Encode(__TEMP_BUFFER, paramA);\
		Encode(__TEMP_BUFFER, paramB);\
		Encode(__TEMP_BUFFER, paramC);\
		__TEMP_NSB->RemoteCallRPC(__TEMP_ID, __TEMP_BUFFER, mode, socket); \
	} \
	else \
		LOG_ERROR("Cannot call function '" #func "' as it is not a registered RPC for the given object"); \
}

/**
* Execute RPC with given settings
* Containing 4 parameters
*/
#define CallRPC_FourParam(socket, mode, object, func, paramA, paramB, paramC, paramD) {\
	NetSerializableBase* __TEMP_NSB = (NetSerializableBase*)object; \
	uint16 __TEMP_ID; \
	if(__TEMP_NSB->FetchRPCIndex(#func, __TEMP_ID)) \
	{ \
		ByteBuffer __TEMP_BUFFER; \
		Encode(__TEMP_BUFFER, paramA);\
		Encode(__TEMP_BUFFER, paramB);\
		Encode(__TEMP_BUFFER, paramC);\
		Encode(__TEMP_BUFFER, paramD);\
		__TEMP_NSB->RemoteCallRPC(__TEMP_ID, __TEMP_BUFFER, mode, socket); \
	} \
	else \
		LOG_ERROR("Cannot call function '" #func "' as it is not a registered RPC for the given object"); \
}

/**
* Execute RPC with given settings
* Containing 5 parameters
*/
#define CallRPC_FiveParam(socket, mode, object, func, paramA, paramB, paramC, paramD, paramE) {\
	NetSerializableBase* __TEMP_NSB = (NetSerializableBase*)object; \
	uint16 __TEMP_ID; \
	if(__TEMP_NSB->FetchRPCIndex(#func, __TEMP_ID)) \
	{ \
		ByteBuffer __TEMP_BUFFER; \
		Encode(__TEMP_BUFFER, paramA);\
		Encode(__TEMP_BUFFER, paramB);\
		Encode(__TEMP_BUFFER, paramC);\
		Encode(__TEMP_BUFFER, paramD);\
		Encode(__TEMP_BUFFER, paramE);\
		__TEMP_NSB->RemoteCallRPC(__TEMP_ID, __TEMP_BUFFER, mode, socket); \
	} \
	else \
		LOG_ERROR("Cannot call function '" #func "' as it is not a registered RPC for the given object"); \
}