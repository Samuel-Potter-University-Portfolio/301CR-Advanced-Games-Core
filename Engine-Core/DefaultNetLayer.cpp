#include "Includes\Core\DefaultNetLayer.h"
#include "Includes\Core\Encoding.h"
#include "Includes\Core\PlayerController.h"
#include "Includes\Core\Game.h"


CLASS_SOURCE(DefaultNetLayer, CORE_API)


DefaultNetLayer::DefaultNetLayer() 
{
	m_connFlags = 0;
	m_password = "";
}

void DefaultNetLayer::OnBegin() 
{
}

void DefaultNetLayer::OnEncodeHandshake(const NetIdentity& host, ByteBuffer& outBuffer)
{
	// Use default controller name
	string playerName = "__PLAYER__";
	OPlayerController* player = GetGame()->GetFirstObject<OPlayerController>();
	if (player != nullptr)
		playerName = player->GetPlayerName();

	Encode<string>(outBuffer, playerName);
	Encode<string>(outBuffer, m_password);
}

NetResponseCode DefaultNetLayer::OnDecodeHandshake(const NetIdentity& connection, ByteBuffer& inBuffer, OPlayerController*& outPlayer)
{
	string playerName;
	string password;

	if (!Decode(inBuffer, playerName) || !Decode(inBuffer, password))
		return NetResponseCode::BadRequest;

	if (!m_password.empty() && m_password != password)
		return NetResponseCode::BadAuthentication;

	outPlayer->SetPlayerName(playerName);
	return NetResponseCode::Accepted;
}

void DefaultNetLayer::SetPassword(const string& pass) 
{
	m_password = pass;

	// Set password bit flag
	if (m_password.empty())
		m_connFlags &= ~LAYER_FLAG_PASSWORD;
	else
		m_connFlags |= LAYER_FLAG_PASSWORD;
}