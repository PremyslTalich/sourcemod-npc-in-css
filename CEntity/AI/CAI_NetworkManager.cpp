
#include "CAI_NetworkManager.h"

CEAI_NetworkManager *g_pAINetworkManager;

CE_LINK_ENTITY_TO_CLASS(CAI_NetworkManager, CEAI_NetworkManager);



//Datamaps
DEFINE_PROP(m_pNetwork,CEAI_NetworkManager);


void CEAI_NetworkManager::InitializeAINetworks()
{
	CEAI_NetworkManager *pNetwork;
	g_pAINetworkManager = pNetwork = dynamic_cast<CEAI_NetworkManager *>(g_helpfunc.FindEntityByClassname(NULL, "ai_network"));
	assert(pNetwork);

	g_pBigAINet = pNetwork->GetNetwork();
}


