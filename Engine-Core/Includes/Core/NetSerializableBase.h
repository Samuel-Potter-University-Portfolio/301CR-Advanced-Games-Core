/**
* Used macros to provide a nice API approach to adding support of RPCs and synced vars
*/
#pragma once
#include "Common.h"
#include "Encoding.h"
#include "ByteBuffer.h"
#include "NetSocket.h"


/**
* The different calling modes avaliable when calling an RPC
*/
enum class RPCCallingMode : uint8 
{
	Unknown		= 0,		
	Host		= 1,	// Execute RPC on host
	Owner		= 2,	// Execute RPC on owning client
	Broadcast	= 3,	// Execute RPC on all clients (Only valid when called by server)
};

/**
* Describes a registered RPC and how to call it
*/
struct RPCInfo 
{
	uint16			index;			// Registered index of this RPC
	RPCCallingMode	callingMode;	// Calling mode of this RPC
	SocketType		socket;			// What socket the RPC call should be sent over
};


/**
* Describes a call request for an RPC
*/
struct RPCRequest 
{
	RPCInfo		function;	// Registered RPC information
	ByteBuffer	params;		// Params to call the function using
};
typedef std::vector<RPCRequest> RPCQueue;

template<>
inline void Encode<RPCRequest>(ByteBuffer& buffer, const RPCRequest& data)
{
	Encode<uint16>(buffer, data.function.index);
	Encode<uint8>(buffer, (uint8)data.function.callingMode);
	Encode<uint16>(buffer, data.params.Size());
	buffer.Push(data.params.Data(), data.params.Size());
}

template<>
inline bool Decode<RPCRequest>(ByteBuffer& buffer, RPCRequest& out, void* context)
{
	uint8 target;
	uint16 paramCount;

	if (!Decode<uint16>(buffer, out.function.index) ||
		!Decode<uint8>(buffer, target) ||
		!Decode<uint16>(buffer, paramCount)
		)
		return false;

	out.function.callingMode = (RPCCallingMode)target;
	buffer.PopBuffer(out.params, paramCount);
	return true;
}



/**
* The different calling modes avaliable when calling an RPC
*/
enum class SyncVarMode : uint8
{
	Unknown		= 0,
	OnChange	= 1,	// Sync whenever the value changes
	Interval	= 2,	// Sync at regular intervals
	Always		= 3,	// Sync every net update
};

/**
* Describes a registered sync var and how to sync it it
*/
struct SyncVarInfo 
{
	uint16			index;			// Registered index of this variable
	SyncVarMode		syncMode;		// Synchronisation mode of this variable
	SocketType		socket;			// What socket the variable should be synced over
};


/**
* Describes a call request for an RPC
*/
struct SyncVarRequest
{
	SyncVarInfo		variable;	// Registered variable information
	ByteBuffer		value;		// Value of this variable
};
typedef std::vector<SyncVarRequest> SyncVarQueue;

template<>
inline void Encode<SyncVarRequest>(ByteBuffer& buffer, const SyncVarRequest& data)
{
	Encode<uint16>(buffer, data.variable.index);
	Encode<uint16>(buffer, data.value.Size());
	buffer.Push(data.value.Data(), data.value.Size());
}

template<>
inline bool Decode<SyncVarRequest>(ByteBuffer& buffer, SyncVarRequest& out, void* context)
{
	uint16 paramCount;

	if (!Decode<uint16>(buffer, out.variable.index) ||
		!Decode<uint16>(buffer, paramCount)
		)
		return false;

	buffer.PopBuffer(out.value, paramCount);
	return true;
}



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
	friend class Game;
	friend class LLevel;

	NetRole m_netRole = NetRole::None;
	uint16 m_networkOwnerId = 0;
	uint16 m_networkId = 0;

	bool bFirstNetUpdate;
	uint16 m_updateCounter = 0;

	RPCQueue m_UdpRpcQueue;
	RPCQueue m_TcpRpcQueue;
	SyncVarQueue m_UdpVarQueue;
	SyncVarQueue m_TcpVarQueue;

protected:
	// Where to check if any value has changed
	std::vector<uint8> m_varCheckValues;

	bool bIsNetSynced = false;

	/**
	* Should the variable at this index be encoded (Used for internval synced vars)
	* @param id			The index of the sync var
	* @returns If this variable should be encoded
	*/
	inline bool ShouldEncodeVar(const uint16& id) const { return ((m_updateCounter + id) % 20) == 0; }
public:
	/**
	* Update this object's role, based on the current session information
	* @param session			The session which is currently active
	* @param assignOwner		Should this object assume the session as it's owner
	*/
	void UpdateRole(const NetSession* session, const bool& assignOwner = false);



public:
	/**
	* Register RPCs in this function
	* @param func			The name of the function
	* @param outInfo		Information about the RPC
	* @returns If the function is registered or not
	*/
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const;

	/**
	* Register variables in this function
	* @param outQueue		The queue to encode any variable changes
	* @param socketType		The socket type this is using
	* @param startIndex		The index to start the counting at
	* @param trackIndex		The index to track changes from from
	* @param forceEncode	Forces all variables to gets encoded
	*/
	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode);
protected:
	/**
	* Call a given function from it's RPC id
	* NOTE: macro order between RegisterRPCs and ExecuteRPC must align
	* @param id				The RPC id of the function
	* @param params			The raw parameters for the function to use
	* @returns If call succeeds
	*/
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params);

	/**
	* Set a variable's value from it's id
	* NOTE: macro order between RegisterSyncVars and ExecuteSyncVar must align
	* @param id				The id of the variable
	* @param value			The raw value for the variable to use
	* @param skipCallbacks	Used during object net creation of clients
	* @returns If call succeeds
	*/
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks);
public:

	/**
	* Enqueue an RPC to be executed by the server, client
	* (If no session is active, they will just execute normally)
	* @param rpcInfo		The RPC to call
	* @param params			Encoded parameters to call the function using
	*/
	void RemoteCallRPC(const RPCInfo& rpcInfo, const ByteBuffer& params);


	/**
	* Has this object got any data to encode
	* @param socketType			The socket type the data will be sent over
	* @returns True if there is pending data
	*/
	inline bool HasQueuedNetData(const SocketType& socketType) const
	{
		if (socketType == TCP)
			return m_TcpRpcQueue.size() != 0 || m_TcpVarQueue.size() != 0;
		else
			return m_UdpRpcQueue.size() != 0 || m_UdpVarQueue.size() != 0;
	}
	/**
	* Clears any net data which is currently queued
	*/
	inline void ClearQueuedNetData()
	{
		m_UdpRpcQueue.clear();
		m_TcpRpcQueue.clear();
		m_UdpVarQueue.clear();
		m_TcpVarQueue.clear();
	}



private:
	/**
	* Callback for just before a net update is about to occur
	*/
	void OnPreNetUpdate();

	/**
	* Encode all currently queued RPC calls
	* @param targetNetId	The net id of where this data will be sent to
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this will be sent over
	*/
	void EncodeRPCRequests(const uint16& targetNetId, ByteBuffer& buffer, const SocketType& socketType);
	/**
	* Decode all RPC calls in this queue
	* @param sourceNetId	The net id of where this data came from
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this was sent over
	*/
	void DecodeRPCRequests(const uint16& sourceNetId, ByteBuffer& buffer, const SocketType& socketType);

	/**
	* Encode all currently queued sync var requests
	* @param targetNetId	The net id of where this data will be sent to
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this will be sent over
	* @param forceEncode	Forcefully encode all variables
	*/
	void EncodeSyncVarRequests(const uint16& targetNetId, ByteBuffer& buffer, const SocketType& socketType, const bool& forceEncode);
	/**
	* Decode all sync var calls in this queue
	* @param sourceNetId	The net id of where this data came from
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this was sent over
	* @param skipCallbacks	Should all var calllbacks be skipped this decode
	*/
	void DecodeSyncVarRequests(const uint16& sourceNetId, ByteBuffer& buffer, const SocketType& socketType, const bool& skipCallbacks);
	

	/**
	* Getters & Setters
	*/
public:
	inline const bool& IsNetSynced() const { return bIsNetSynced; }

	/**
	* A unique ID for identifying this object over the current NetSession
	*/
	inline const uint16& GetNetworkID() const { return m_networkId; }
	inline const uint16& GetNetworkOwnerID() const { return m_networkOwnerId; }

	inline const NetRole& GetNetRole() const { return m_netRole; }
	inline const bool IsNetOwner() const { return m_netRole == NetRole::None || m_netRole == NetRole::HostOwner || m_netRole == NetRole::RemoteOwner; }
	inline const bool IsNetHost() const { return m_netRole == NetRole::HostOwner || m_netRole == NetRole::HostPuppet; }
	inline const bool HasNetControl() const { return IsNetOwner() || IsNetHost(); }
};



/**
* Placed at the start of RegisterRPCs to create temporary vars (To avoid naming problems)
* and to handle parent calls correctly
*/
#define RPC_INDEX_HEADER(func, outInfo) \
	const char*& __TEMP_NAME = func; \
	RPCInfo& __TEMP_INFO = outInfo; \
	if(__super::RegisterRPCs(__TEMP_NAME, __TEMP_INFO)) return true; 

/**
* Placed after RPC_INDEX_HEADER in ExecuteRPC to create an entry for a function
*/
#define RPC_INDEX(socketType, mode, func) \
	if (std::strcmp(__TEMP_NAME, #func) == 0) \
	{ \
		__TEMP_INFO.callingMode = mode; \
		__TEMP_INFO.socket = socketType; \
		return true; \
	} \
	else \
		++__TEMP_INFO.index;


/**
* Placed at the start of RegisterSyncVars to create temporary vars (To avoid naming problems)
* and to handle parent calls correctly
*/
#define SYNCVAR_INDEX_HEADER(queue, socketType, index, trackIndex, forceEncode) \
	SyncVarQueue& __TEMP_QUEUE = queue; \
	const SocketType& __TEMP_SOCKET = socketType; \
	uint16& __TEMP_INDEX = index; \
	uint32& __TEMP_TRACK = trackIndex; \
	const bool& __TEMP_FORCE_ENCODE = forceEncode; \
	__super::RegisterSyncVars(__TEMP_QUEUE, __TEMP_SOCKET, __TEMP_INDEX, __TEMP_TRACK, __TEMP_FORCE_ENCODE);

/**
* Placed after SYNCVAR_INDEX_HEADER in ExecuteSyncVar to create an entry for a variable
*/
#define SYNCVAR_INDEX(socketType, mode, type, var) \
	if (__TEMP_SOCKET == socketType) \
		{ \
			if (__TEMP_FORCE_ENCODE || mode == SyncVarMode::Always || (mode == SyncVarMode::Interval && ShouldEncodeVar(__TEMP_INDEX))) \
			{ \
				SyncVarRequest request; \
				request.variable.index = __TEMP_INDEX; \
				request.variable.socket = socketType; \
				request.variable.syncMode = mode; \
				Encode(request.value, var); \
				__TEMP_QUEUE.emplace_back(request); \
				\
				if(__TEMP_FORCE_ENCODE && mode == SyncVarMode::OnChange) \
				{ \
					type* vptr; \
					if (typeid(type) == typeid(string)) \
					{ \
						if (m_varCheckValues.size() < __TEMP_TRACK + STR_MAX_ENCODE_LEN) \
							m_varCheckValues.resize(__TEMP_TRACK + STR_MAX_ENCODE_LEN); \
						\
						*(m_varCheckValues.data() + __TEMP_TRACK + STR_MAX_ENCODE_LEN - 1) = '\0'; \
						vptr = (type*)(m_varCheckValues.data() + __TEMP_TRACK); \
						__TEMP_TRACK += STR_MAX_ENCODE_LEN; \
					} \
					else \
					{ \
						if (m_varCheckValues.size() < __TEMP_TRACK + sizeof(type)) \
							m_varCheckValues.resize(__TEMP_TRACK + sizeof(type)); \
						\
						vptr = (type*)(m_varCheckValues.data() + __TEMP_TRACK); \
						__TEMP_TRACK += sizeof(type); \
					} \
					*vptr = var; \
					\
				} \
			} \
			else if (mode == SyncVarMode::OnChange) \
			{ \
				type* vptr; \
				if (typeid(type) == typeid(string)) \
				{ \
					if (m_varCheckValues.size() < __TEMP_TRACK + STR_MAX_ENCODE_LEN) \
						m_varCheckValues.resize(__TEMP_TRACK + STR_MAX_ENCODE_LEN); \
					\
					*(m_varCheckValues.data() + __TEMP_TRACK + STR_MAX_ENCODE_LEN - 1) = '\0'; \
					vptr = (type*)(m_varCheckValues.data() + __TEMP_TRACK); \
					__TEMP_TRACK += STR_MAX_ENCODE_LEN; \
				} \
				else \
				{ \
					if (m_varCheckValues.size() < __TEMP_TRACK + sizeof(type)) \
						m_varCheckValues.resize(__TEMP_TRACK + sizeof(type)); \
					\
					vptr = (type*)(m_varCheckValues.data() + __TEMP_TRACK); \
					__TEMP_TRACK += sizeof(type); \
				} \
				\
				if (*vptr != var) \
				{ \
					*vptr = var; \
					SyncVarRequest request; \
					request.variable.index = __TEMP_INDEX; \
					request.variable.socket = socketType; \
					request.variable.syncMode = mode; \
					Encode(request.value, var); \
					__TEMP_QUEUE.emplace_back(request); \
				} \
			} \
		} \
			++__TEMP_INDEX;



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
* Placed at the start of ExecuteSyncVar to create temporary vars (To avoid naming problems)
* and to handle parent calls correctly
*/
#define SYNCVAR_EXEC_HEADER(id, value, skipCallbacks) \
	uint16 __TEMP_ID = id; \
	ByteBuffer& __TEMP_BUFFER = value; \
	const bool& __TEMP_SKIP_CALLBACKS = skipCallbacks; \
	if(__super::ExecuteSyncVar(__TEMP_ID, __TEMP_BUFFER, __TEMP_SKIP_CALLBACKS)) return true;

/**
* Execution for a variable at the placed index
*/
#define SYNCVAR_EXEC(var) \
	if (__TEMP_ID == 0) \
		return Decode(__TEMP_BUFFER, var); \
	else \
		--__TEMP_ID;

/**
* Execution for a variable at the placed index
* Calls the callback function when the value is changed (Over the net)
*/
#define SYNCVAR_EXEC_Callback(var, callback) \
	if (__TEMP_ID == 0) \
	{ \
		const bool decoded = Decode(__TEMP_BUFFER, var); \
		if(!decoded) return false; \
		if(!__TEMP_SKIP_CALLBACKS) callback(); \
		return true; \
	} \
	else \
		--__TEMP_ID;



/**
* GENERIC* Execute RPC with given settings
*/
#define __CallRPC(object, func, funcCall, funcEncode) \
{ \
	NetSerializableBase* __TEMP_NSB = static_cast<NetSerializableBase*>(object); \
	RPCInfo __TEMP_INFO; \
	if (__TEMP_NSB->RegisterRPCs(#func, __TEMP_INFO)) \
	{ \
		if(GetNetworkID() != 0) \
		{ \
			if ((__TEMP_NSB->IsNetHost() && __TEMP_INFO.callingMode == RPCCallingMode::Host) || (__TEMP_NSB->IsNetOwner() && __TEMP_INFO.callingMode == RPCCallingMode::Owner)) \
				object->funcCall; \
			else if (__TEMP_NSB->IsNetHost() || __TEMP_INFO.callingMode != RPCCallingMode::Broadcast) \
			{ \
				ByteBuffer __TEMP_BUFFER; \
				funcEncode \
				__TEMP_NSB->RemoteCallRPC(__TEMP_INFO, __TEMP_BUFFER); \
			} \
			else \
				LOG_ERROR("Invalid rights to call function '" #func "'"); \
		} \
	} \
	else \
		LOG_ERROR("Cannot call function '" #func "' as it is not a registered RPC for the given object"); \
}

/**
* Execute RPC with given settings
* Containing 0 parameters
*/
#define CallRPC(object, func) __CallRPC(object, func, func(),)

/**
* Execute RPC with given settings
* Containing 1 parameters
*/
#define CallRPC_OneParam(object, func, paramA) __CallRPC(object, func, func(paramA), Encode(__TEMP_BUFFER,paramA);)

/**
* Execute RPC with given settings
* Containing 2 parameters
*/
#define CallRPC_TwoParam(object, func, paramA, paramB) __CallRPC(object, func, func(paramA, paramB), Encode(__TEMP_BUFFER,paramA);Encode(__TEMP_BUFFER,paramB);)

/**
* Execute RPC with given settings
* Containing 3 parameters
*/
#define CallRPC_ThreeParam(object, func, paramA, paramB, paramC) __CallRPC(object, func, func(paramA, paramB, paramC), Encode(__TEMP_BUFFER,paramA);Encode(__TEMP_BUFFER,paramB);Encode(__TEMP_BUFFER,paramC);)

/**
* Execute RPC with given settings
* Containing 4 parameters
*/
#define CallRPC_FourParam(object, func, paramA, paramB, paramC, paramD) __CallRPC(object, func, func(paramA, paramB, paramC, paramD), Encode(__TEMP_BUFFER,paramA);Encode(__TEMP_BUFFER,paramB);Encode(__TEMP_BUFFER,paramC);Encode(__TEMP_BUFFER,paramD);)

/**
* Execute RPC with given settings
* Containing 5 parameters
*/
#define CallRPC_FiveParam(object, func, paramA, paramB, paramC, paramD, paramE) __CallRPC(object, func, func(paramA, paramB, paramC, paramD, paramE), Encode(__TEMP_BUFFER,paramA);Encode(__TEMP_BUFFER,paramB);Encode(__TEMP_BUFFER,paramC);Encode(__TEMP_BUFFER,paramD);Encode(__TEMP_BUFFER,paramE);)
