#ifndef	AI_NETWORKMANAGER_H
#define	AI_NETWORKMANAGER_H

#include "CEntity.h"
#include "CAI_Network.h"

class CEAI_NetworkManager : public CPointEntity
{
public:
	CE_DECLARE_CLASS(CEAI_NetworkManager, CPointEntity);

	static void		InitializeAINetworks();
public:
	//CEAI_NetworkManager(void);
	//virtual ~CEAI_NetworkManager(void);
	
	//void			Spawn ();
	//virtual	int		ObjectCaps( void ) { return BaseClass::ObjectCaps() | FCAP_DONT_SAVE; }
	void			RebuildNetworkGraph();			// Used in WC edit mode
	void			StartRebuild();			// Used in WC edit mode
	void			LoadNetworkGraph();

	static bool		NetworksLoaded()	{ return gm_fNetworksLoaded; }
//	bool			IsInitialized()	{ return m_fInitalized; }

	void			BuildNetworkGraph();

	static void		DeleteAllAINetworks();

	void			FixupHints();
	void			MarkDontSaveGraph();

public:
	//CAI_NetworkEditTools *	GetEditOps() { return m_pEditOps; }
	CAI_Network *			GetNetwork() { return m_pNetwork; }
	
private:
	
	void			DelayedInit();
	void			RebuildThink();
	void			SaveNetworkGraph( void) ;	
	static bool		IsAIFileCurrent( const char *szMapName );		
	
	static bool				gm_fNetworksLoaded;							// Have AINetworks been loaded
	

protected: // Datamaps
	DECLARE_DATAMAP_OFFSET(CAI_Network *, m_pNetwork,0x308);


};



#endif
