#include "Includes\Core\NetSession.h"
#include "Includes\Core\NetController.h"

#include "Includes\Core\Engine.h"
#include "Includes\Core\Logger.h"



NetSession::NetSession(const Engine* engine, const NetIdentity identity) :
	m_engine(engine), m_netIdentity(identity)
{
}

NetSession::~NetSession()
{
}

void NetSession::HandleUpdate(const float& deltaTime)
{
	// Only tick at desired rate
	m_tickTimer -= deltaTime;
	if (m_tickTimer > 0)
		return;
	m_tickTimer += m_sleepRate;

	Update(deltaTime);
}
