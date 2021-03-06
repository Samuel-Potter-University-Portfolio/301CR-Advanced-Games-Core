#include "Includes\Core\NetLayer.h"


CLASS_SOURCE(NetLayer, CORE_API)


void NetLayer::Initialize(Game* game, NetSession* session, ConfigLayer configLayer)
{
	m_game = game;
	m_session = session;
	if (configLayer)
		configLayer(this);
	OnBegin();
}

NetLayer::~NetLayer()
{
}

void NetLayer::OnEncodeHandshake(const NetIdentity& host, ByteBuffer& outBuffer) 
{
}

NetResponseCode NetLayer::OnDecodeHandshake(const NetIdentity& connection, ByteBuffer& inBuffer, OPlayerController*& outPlayer) 
{
	// Don't do anything extra by default
	return NetResponseCode::Accepted;
}