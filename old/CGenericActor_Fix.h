#ifndef _INCLUDE_CGENEROCATOR_FIX_H_
#define _INCLUDE_CGENEROCATOR_FIX_H_

#include "CCycler_Fix.h"
#include "CAI_Behavior.h"


abstract_class CE_CGenericActor_Fix : public CE_Cycler_Fix
{
public:
	CE_DECLARE_CLASS(CE_CGenericActor_Fix, CE_Cycler_Fix);
	CE_CUSTOM_ENTITY();

public: // Fixed function
	virtual void CE_PostInit();
	virtual void Spawn(void);
	virtual void Precache(void);
	virtual float MaxYawSpeed();
	virtual Class_T Classify( void );
	virtual int	GetSoundInterests ( void );
	
	virtual bool CreateBehaviors();

public:
	void	AddBehavior( CAI_BehaviorBase *pBehavior );
	bool	BehaviorSelectSchedule();
	bool 	IsRunningBehavior() const;

};


#endif

