#ifndef _CH_AUTO_H_
#define _CH_AUTO_H_

#include <fsm.h>
#include <fsmsystem.h>

#include "../kernel/stdMsgpc16pl16.h"
#include "NetFSM.h"


class ChAuto : public FiniteStateMachine {
	
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
	enum	ChStates {	FSM_Ch_Idle, 
						FSM_Ch_Connecting, 
						FSM_Ch_Connected };

	//FSM_Ch_Idle
	void	FSM_Ch_Idle_Cl_Connection_Request();
	//FSM_Ch_Connecting
	void	FSM_Ch_Connecting_TIMER1_EXPIRED();
	void	FSM_Ch_Connecting_Sock_Connection_Acccept();
	//FSM_Ch_Connected
	void	FSM_Ch_Connected_Cl_MSG();
	void	FSM_Ch_Connected_Sock_MSG();
	void	FSM_Ch_Connected_Sock_Disconected();
		
public:
	ChAuto();
	~ChAuto();
	
	//bool FSMMsg_2_NetMsg();
	void NetMsg_2_FSMMsg(const char* apBuffer, uint16 anBufferLength);

	void Initialize();

protected:
	static DWORD WINAPI ClientListener(LPVOID);
	
	SOCKET m_Socket;
	HANDLE m_hThread;
	DWORD m_nThreadID;
	uint16 m_nMaxMsgSize;
};

#endif /* _CH_AUTO_H */