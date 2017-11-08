#include "Includes\Core\NetHostSession.h"
#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\Level.h"


NetHostSession::NetHostSession(const Engine* engine, const NetIdentity identity) :
	NetSession(engine, identity)
{
	bIsHost = true;
}

NetHostSession::~NetHostSession()
{
	for (NetClient* c : m_clients)
		delete c;

	LOG("NetHostSession closed.");
}

bool NetHostSession::Start() 
{
	const NetIdentity& host = GetSessionIdentity();

	if (!m_TcpSocket.Listen(host))
	{
		LOG_ERROR("Unable to open net session on (%s:%i). TCP socket not openned (Maybe the port is already in use).", host.ip.toString().c_str(), host.port);
		return false;
	}
	if (!m_UdpSocket.Listen(host))
	{
		LOG_ERROR("Unable to open net session on (%s:%i). UDP socket not openned (Maybe the port is already in use).", host.ip.toString().c_str(), host.port);
		return false;
	}

	bIsConnected = true;

	LOG("Host net session openned on (%s:%i)", host.ip.toString().c_str(), host.port);
	return true;
}

void NetHostSession::Update(const float& deltaTime)
{
	// Read from remote
	{
		std::vector<RawNetPacket> packets;

		// Fetch TCP packets
		if (m_TcpSocket.Poll(packets))
		{
			for (RawNetPacket& p : packets)
			{
				p.buffer.Flip();
				ForwardToClient(p, m_TcpSocket);
			}
		}

		// Fetch UDP packets
		packets.clear();
		if (m_UdpSocket.Poll(packets))
		{
			// Fetch UDP packets
			for (RawNetPacket& p : packets)
			{
				p.buffer.Flip();
				ForwardToClient(p, m_UdpSocket);
			}
		}
	}

	
	// Encode and broadcast net update
	{
		ByteBuffer tcpContent;
		ByteBuffer udpContent;
		m_engine->GetGame()->PerformNetEncode(tcpContent, SocketType::TCP);
		m_engine->GetGame()->PerformNetEncode(udpContent, SocketType::UDP);

		// Send out packet update
		for (NetClient* client : m_clients)
		{
			const NetIdentity& identity = client->GetNetIdentity();
			if (!m_TcpSocket.SendTo(tcpContent.Data(), tcpContent.Size(), identity))
			{
				// TODO - Handle disconnections 
				LOG_ERROR("Failed to send TCP update to %s:%i", identity.ip.toString(), identity.port);
			}
			if (!m_UdpSocket.SendTo(udpContent.Data(), udpContent.Size(), identity))
				LOG_ERROR("Failed to send UDP update to %s:%i", identity.ip.toString(), identity.port);
		}
	}
}

NetResponseCode NetHostSession::VerifyHandshake(NetClient* client, RawNetPacket& packet, NetSocket& socket)
{
	// Check message header
	uint16 requestType;
	{
		Version engineVersion;
		Version gameVersion;

		// Invalid header
		if (!Decode<Version>(packet.buffer, engineVersion) ||
			!Decode<Version>(packet.buffer, gameVersion) ||
			!Decode<uint16>(packet.buffer, requestType))
		{
			ByteBuffer response;
			Encode<uint16>(response, (uint16)NetResponseCode::BadRequest);
			socket.SendTo(response.Data(), response.Size(), packet.source);
			return NetResponseCode::BadRequest;
		}

		// Invalid versions
		if (m_engine->GetVersionNo() != engineVersion || m_engine->GetGame()->GetVersionNo() != gameVersion)
		{
			ByteBuffer response;
			Encode<uint16>(response, (uint16)NetResponseCode::BadVersions);
			socket.SendTo(response.Data(), response.Size(), packet.source);
			return NetResponseCode::BadVersions;
		}

		// TODO - Ban checks
		// TODO - Password checks
		// TODO - Whitelist checks
		// TODO - Server is full checks
	}


	// Perform any additional checks/payload filling
	ByteBuffer response;
	NetRequestType request = (NetRequestType)requestType;

	switch (request)
	{
	// User is just pinging the server to get a response
	case NetRequestType::Ping:
		Encode<uint16>(response, (uint16)NetResponseCode::Responded);

		socket.SendTo(response.Data(), response.Size(), packet.source);
		return NetResponseCode::Responded;


	// The user is attempting to connect to the server as a player
	case NetRequestType::Connect:
		Encode<uint16>(response, (uint16)NetResponseCode::Accepted);

		socket.SendTo(response.Data(), response.Size(), packet.source);
		return NetResponseCode::Accepted;


	// The user is querying for server details
	case NetRequestType::Query:
		Encode<uint16>(response, (uint16)NetResponseCode::Responded);
		Encode<uint16>(response, m_clients.size());		// Players connected 
		Encode<uint16>(response, maxPlayerCount);		// Player limit
		Encode<string>(response, "Unnamed Server");		// TODO - Server name
		Encode<uint8>(response, 0);						// TODO - Bitflags

		socket.SendTo(response.Data(), response.Size(), packet.source);
		return NetResponseCode::Responded;
	}

	return NetResponseCode::Unknown;
}

NetClient* NetHostSession::GetClient(const NetIdentity& identity) 
{
	NetClient* client = m_clientLookup[identity];
	if (client != nullptr)
		return client;

	return nullptr;
}

void NetHostSession::ForwardToClient(RawNetPacket& packet, NetSocket& socket) 
{
	const NetIdentity identity = packet.source;
	NetClient* client = GetClient(identity);
	

	// Perform handshake verification
	if (client == nullptr && socket.GetSocketType() == SocketType::TCP)
	{
		client = new NetClient(identity);

		// If hand shake succeeds, track client
		if (VerifyHandshake(client, packet, socket) == NetResponseCode::Accepted)
		{
			m_clients.emplace_back(client);
			m_clientLookup[identity] = client;
			LOG("Client connected from %s:%i", identity.ip.toString().c_str(), identity.port);
		}
		else
			delete client;
		return;
	}


	// Invalid client (Probably spam/invalid traffic)
	if (client == nullptr)
		return;


	// TODO - Actually do stuff with the packets
}
