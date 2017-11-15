#include "ConnDudLevel.h"


CLASS_SOURCE(LConnDudLevel)


LConnDudLevel::LConnDudLevel()
{
}

void LConnDudLevel::OnBuildLevel() 
{
	GetGame()->GetEngine()->GetNetController()->JoinSession(GetGame()->GetEngine()->GetDefaultNetIdentity());
}