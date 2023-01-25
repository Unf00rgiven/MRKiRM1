#ifndef _User_AUTO_H_
#define _User_AUTO_H_

#include <fsm.h>
#include <fsmsystem.h>

#include "../kernel/stdMsgpc16pl16.h"
#include "NetFSM.h"


class UserAuto : public FiniteStateMachine {
	
	// for FSM
	StandardMessage StandardMsgCoding;
	
	MessageInterface *GetMessageInterface(uint32 id);
	void	SetDefaultHeader(uint8 infoCoding);
	void	SetDefaultFSMData();
	void	NoFreeInstances();
	void	Reset();
	uint8	GetMbxId();
	uint8	GetAutomate();
	uint32	GetObject();
	void	ResetData();
	
	// FSM States
	enum	UserStates {FSM_User_Idle, 
						FSM_User_Connecting, 
						FSM_User_Connected };

	
public:
	UserAuto();
	~UserAuto();
	
	void Initialize();
	
	void FSM_User_Idle_Set_All();
	void FSM_User_Connecting_User_Connected();
	void FSM_User_Connecting_User_Connecton_Fail();
	void FSM_User_Connected_Mail();
	void FSM_User_Connected_User_Save_Mail();
	void FSM_User_Connected_User_Disconnected();

	void Start();
	
protected:
	
	FILE* m_File;
	char m_FileName[256];
	
};

#endif /* _User_AUTO_H */