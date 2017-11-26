#include "BPlayerController.h"
#include "ChatWidget.h"


CLASS_SOURCE(OBPlayerController)


OBPlayerController::OBPlayerController()
{
}

void OBPlayerController::OnBegin()
{
	Super::OnBegin();

	if (UChatWidget::s_main != nullptr)
		UChatWidget::s_main->LogMessage(nullptr, GetDisplayName() + " has connected.");
}

void OBPlayerController::OnDestroy() 
{
	Super::OnDestroy();
	if (UChatWidget::s_main != nullptr)
		UChatWidget::s_main->LogMessage(nullptr, GetDisplayName() + " has disconnected.");
}

bool OBPlayerController::RegisterRPCs(const char* func, RPCInfo& outInfo) const 
{
	RPC_INDEX_HEADER(func, outInfo);
	RPC_INDEX(TCP, RPCCallingMode::Host, SendMessage);
	RPC_INDEX(TCP, RPCCallingMode::Broadcast, BroadcastMessage);
	return false;
}

bool OBPlayerController::ExecuteRPC(uint16& id, ByteBuffer& params) 
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC_OneParam(SendMessage, string);
	RPC_EXEC_OneParam(BroadcastMessage, string);
	return false;
}


void OBPlayerController::SendMessage(const string& message) 
{
	LOG("[%s]: %s", GetPlayerName().c_str(), message.c_str());
	CallRPC_OneParam(this, BroadcastMessage, message);
}

void OBPlayerController::BroadcastMessage(const string& message) 
{
	if (UChatWidget::s_main != nullptr)
		UChatWidget::s_main->LogMessage(this, message);
}


string OBPlayerController::GetDisplayName() const 
{
	// Clamp player name, if too long
	const string playerName = GetPlayerName();

	const uint32 maxNameSize = 24;
	if (playerName.size() >= maxNameSize)
		return playerName.substr(0, maxNameSize - 2) + "..";
	else
		return playerName;
}