#include "BPlayerController.h"
#include "ChatWidget.h"
#include "LobbyController.h"


CLASS_SOURCE(OBPlayerController)
const std::vector<Colour> OBPlayerController::s_supportedColours(
{

	Colour(255,0,0), // Hue 0
	Colour(24,255,0), // Hue 112
	Colour(0,162,255), // Hue 202
	Colour(221,0,255), // Hue 292

	Colour(255,255,255), // White

	Colour(255,94,0), // Hue 22
	Colour(0,255,64), // Hue 135
	Colour(0,64,255), // Hue 225
	Colour(255,0,191), // Hue 315

	Colour(225,255,0), // Hue 67
	Colour(0,255,157), // Hue 157
	Colour(30,0,255), // Hue 247
	Colour(255,0,98), // Hue 337


	Colour(128,255,0), // Hue 90
	Colour(0,255,255), // Hue 180
	Colour(128,0,255), // Hue 270

	Colour(0,0,0) // Black for out of bounds indicies
}
);

std::queue<uint32> OBPlayerController::s_colourQueue({ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 });


OBPlayerController::OBPlayerController()
{
}

void OBPlayerController::OnBegin()
{
	Super::OnBegin();


	// Assign a colour if haven't yet
	if ((IsNetHost() || GetNetRole() == NetRole::None) && m_colourIndex == 16)
		AssignColour();


	if (UChatWidget::s_main != nullptr)
		UChatWidget::s_main->LogMessage(nullptr, GetDisplayName() + " has connected.");
}

void OBPlayerController::OnDestroy() 
{
	Super::OnDestroy();


	// Free up colour
	if (IsNetHost())
	{
		if (m_colourIndex != 16)
			s_colourQueue.push(m_colourIndex);
	}


	if (UChatWidget::s_main != nullptr)
		UChatWidget::s_main->LogMessage(nullptr, GetDisplayName() + " has disconnected.");
}

bool OBPlayerController::RegisterRPCs(const char* func, RPCInfo& outInfo) const 
{
	RPC_INDEX_HEADER(func, outInfo);
	RPC_INDEX(TCP, RPCCallingMode::Host, SendMessage);
	RPC_INDEX(TCP, RPCCallingMode::Broadcast, BroadcastMessage);
	RPC_INDEX(TCP, RPCCallingMode::Host, SetReady_Host);
	RPC_INDEX(TCP, RPCCallingMode::Host, SetMapVote_Host);
	return false;
}
bool OBPlayerController::ExecuteRPC(uint16& id, ByteBuffer& params) 
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC_OneParam(SendMessage, string);
	RPC_EXEC_OneParam(BroadcastMessage, string);
	RPC_EXEC_OneParam(SetReady_Host, bool);
	RPC_EXEC_OneParam(SetMapVote_Host, uint32);
	return false;
}

void OBPlayerController::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) 
{
	SYNCVAR_INDEX_HEADER(outQueue, socketType, index, trackIndex, forceEncode);
	SYNCVAR_INDEX(TCP, SyncVarMode::OnChange, uint32, m_colourIndex);
	SYNCVAR_INDEX(TCP, SyncVarMode::OnChange, bool, bIsReady);
	SYNCVAR_INDEX(TCP, SyncVarMode::OnChange, ABCharacter*, m_character);
}
bool OBPlayerController::ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) 
{
	SYNCVAR_EXEC_HEADER(id, value, skipCallbacks);
	SYNCVAR_EXEC(m_colourIndex);
	SYNCVAR_EXEC(bIsReady);
	SYNCVAR_EXEC(m_character);
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

void OBPlayerController::AssignColour()
{
	// Already assigned colour, so don't repeat
	if (m_colourIndex != 16)
		return;

	if (s_colourQueue.size() == 0)
		m_colourIndex = 16;
	else
	{
		m_colourIndex = s_colourQueue.front();
		s_colourQueue.pop();
	}
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

void OBPlayerController::SetMapVote_Host(const uint32& index)
{
	LLevel* level = GetGame()->GetCurrentLevel();
	ALobbyController* controller = level != nullptr ? dynamic_cast<ALobbyController*>(level->GetLevelController()) : nullptr;
	if (controller != nullptr) controller->UpdateMapVote(this, index);
}