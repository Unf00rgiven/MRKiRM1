#include <stdio.h>

#include "const.h"
#include "ChAuto.h"

#define StandardMessageCoding 0x00

ChAuto::ChAuto() : FiniteStateMachine(CH_AUTOMATE_TYPE_ID, CH_AUTOMATE_MBX_ID, 1, 3, 3) {
}

ChAuto::~ChAuto() {
}

uint8 ChAuto::GetAutomate() {
	return CH_AUTOMATE_TYPE_ID;
}

/* This function actually connnects the ChAuto with the mailbox. */
uint8 ChAuto::GetMbxId() {
	return CH_AUTOMATE_MBX_ID;
}

uint32 ChAuto::GetObject() {
	return GetObjectId();
}

MessageInterface *ChAuto::GetMessageInterface(uint32 id) {
	return &StandardMsgCoding;
}

void ChAuto::SetDefaultHeader(uint8 infoCoding) {
	SetMsgInfoCoding(infoCoding);
	SetMessageFromData();
}

void ChAuto::SetDefaultFSMData() {
	SetDefaultHeader(StandardMessageCoding);
}

void ChAuto::NoFreeInstances() {
	printf("[%d] ChAuto::NoFreeInstances()\n", GetObjectId());
}

void ChAuto::Reset() {
	printf("[%d] ChAuto::Reset()\n", GetObjectId());
}


void ChAuto::Initialize() {
	SetState(FSM_Ch_Idle);	
	
	//intitialization message handlers
	InitEventProc(FSM_Ch_Idle ,MSG_Connection_Request, (PROC_FUN_PTR)&ChAuto::FSM_Ch_Idle_Cl_Connection_Request );
	InitEventProc(FSM_Ch_Connecting ,TIMER1_EXPIRED, (PROC_FUN_PTR)&ChAuto::FSM_Ch_Connecting_TIMER1_EXPIRED );
	InitEventProc(FSM_Ch_Connecting ,MSG_Sock_Connection_Acccept, (PROC_FUN_PTR)&ChAuto::FSM_Ch_Connecting_Sock_Connection_Acccept );
	InitEventProc(FSM_Ch_Connected ,MSG_Cl_MSG, (PROC_FUN_PTR)&ChAuto::FSM_Ch_Connected_Cl_MSG );
	InitEventProc(FSM_Ch_Connected ,MSG_Sock_MSG, (PROC_FUN_PTR)&ChAuto::FSM_Ch_Connected_Sock_MSG );
	InitEventProc(FSM_Ch_Connected ,MSG_Sock_Disconected, (PROC_FUN_PTR)&ChAuto::FSM_Ch_Connected_Sock_Disconected );

	InitTimerBlock(TIMER1_ID, TIMER1_COUNT, TIMER1_EXPIRED);
}


void VignerEncryption(char* msg)
{
	// KEY = "KLJUC"
	int key[5] = { 10 , 11, 9, 20, 3 };
	int i = 0, k = 0;
	int tmp1 = 0, tmp2 = 0;

	while (msg[i] != '\0')
	{
		if (msg[i] == '+') msg[i] = 'O';
		else if (msg[i] == '-') msg[i] = 'G';
		else if (msg[i] == ' ') msg[i] = 'I';
		else
		{

			// LOWER CASE
			if (msg[i] >= 'a' && msg[i] <= 'z')
			{
				// OUT OF RANGE
				tmp2 = msg[i] + key[k];
				tmp1 = tmp2 - 'z' - 1;
				if (tmp2 > 'z')
				{
					msg[i] = 'a' + tmp1;
				}
				else
				{
					msg[i] = tmp2;
				}

			}

			// UPPER CASE
			if (msg[i] >= 'A' && msg[i] <= 'Z')
			{
				// OUT OF RANGE
				tmp2 = msg[i] + key[k];
				tmp1 = tmp2 - 'Z' - 1;
				if (tmp2 > 'Z')
				{
					msg[i] = 'A' + tmp1;
				}
				else
				{
					msg[i] = tmp2;
				}
			}
		}

		i++;
		k = ++k % 5;

	}
}

void VignerDencryption(char* msg)
{
	// KEY = "KLJUC"
	int key[5] = { 10 , 11, 9, 20, 3 };
	int i = 0, k = 0;
	int tmp1 = 0, tmp2 = 0;

	while (msg[i] != '\0')
	{
		if (msg[i] == 'O') msg[i] = '+';
		else if (msg[i] == 'G') msg[i] = '-';
		else if (msg[i] == 'I') msg[i] = ' ';
		else
		{

			// LOWER CASE
			if (msg[i] >= 'a' && msg[i] <= 'z')
			{
				// OUT OF RANGE
				tmp2 = msg[i] - key[k];
				tmp1 = 'a' - tmp2 - 1;
				if (tmp2 < 'a')
				{
					msg[i] = 'z' - tmp1;
				}
				else
				{
					msg[i] = tmp2;
				}

			}

			// UPPER CASE
			if (msg[i] >= 'A' && msg[i] <= 'Z')
			{
				// OUT OF RANGE
				tmp2 = msg[i] - key[k];
				tmp1 = 'A' - tmp2 - 1;
				if (tmp2 < 'A')
				{
					msg[i] = 'Z' - tmp1;
				}
				else
				{
					msg[i] = tmp2;
				}
			}
		}

		i++;
		k = ++k % 5;

	}
}

void ChAuto::FSM_Ch_Idle_Cl_Connection_Request(){

	StartTimer(TIMER1_ID);
	
	SetState(FSM_Ch_Connecting);
	
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0) 
	{
		return;
	} 

	/* Try to resolve the server name. */
	sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
	
	unsigned int addr = inet_addr(ADRESS);
	if (addr != INADDR_NONE) 
	{
        server_addr.sin_addr.s_addr	= addr;
        server_addr.sin_family		= AF_INET;
    }
    else 
	{
        hostent* hp = gethostbyname(ADRESS);
        if (hp != 0)  
		{
            memcpy( &(server_addr.sin_addr), hp->h_addr, hp->h_length );
            server_addr.sin_family = hp->h_addrtype;
        }
        else 
		{
			return ;
        }
    }
	server_addr.sin_port = htons(PORT);

	/* Create the socket. */ 
	m_Socket = socket(PF_INET, SOCK_STREAM, 0);
	if (m_Socket == INVALID_SOCKET) {
		return ;
	}

	/* Try to reach the server. */
	if (connect(m_Socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
		/* Here some additional cleanup should be done. */
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return ;
	}



	/* Then, start the thread that will listen on the the newly created socket. */
	m_hThread = CreateThread(NULL, 0, ClientListener, (LPVOID) this, 0, &m_nThreadID); 
	if (m_hThread == NULL) {
		/* Cannot create thread.*/
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return ;
	}

	
	
}

void ChAuto::FSM_Ch_Connecting_TIMER1_EXPIRED(){

	PrepareNewMessage(0x00, MSG_Cl_Connection_Reject);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);

	SetState(FSM_Ch_Idle);

}

void ChAuto::FSM_Ch_Connecting_Sock_Connection_Acccept(){

	PrepareNewMessage(0x00, MSG_Cl_Connection_Accept);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);

	StopTimer(TIMER1_ID);

	SetState(FSM_Ch_Connected);

}

void ChAuto::FSM_Ch_Connected_Cl_MSG(){

	char* data = new char[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data,buffer + 4,size);

	data[size] = 0;
	printf("SENT: %s", data);
	VignerEncryption(data + 5);

	if (send(m_Socket, data, size, 0) != size) {
		delete [] data;
	} else {
		delete [] data;
	}

}

void ChAuto::FSM_Ch_Connected_Sock_MSG(){

}

void ChAuto::FSM_Ch_Connected_Sock_Disconected(){

	PrepareNewMessage(0x00, MSG_Cl_Disconected);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(CL_AUTOMATE_MBX_ID);

	SetState(FSM_Ch_Idle);

}

void ChAuto::NetMsg_2_FSMMsg(const char* apBuffer, uint16 anBufferLength) {
	
	PrepareNewMessage(0x00, MSG_MSG);
	SetMsgToAutomate(CL_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	AddParam(PARAM_DATA,anBufferLength,(uint8 *)apBuffer);
	SendMessage(CL_AUTOMATE_MBX_ID);
	
}

DWORD ChAuto::ClientListener(LPVOID param) {
	ChAuto* pParent = (ChAuto*) param;
	int nReceivedBytes;
	char* buffer = new char[255];

	nReceivedBytes = recv(pParent->m_Socket, buffer, 255, 0);
	if (nReceivedBytes < 0) {
			DWORD err = WSAGetLastError();
	}else{
		pParent->FSM_Ch_Connecting_Sock_Connection_Acccept();
		
		/* Receive data from the network until the socket is closed. */ 
		do {
			nReceivedBytes = recv(pParent->m_Socket, buffer, 255, 0);
			VignerDencryption(buffer);
			if (nReceivedBytes == 0)
			{
				printf("Disconnected from server!\n");
				pParent->FSM_Ch_Connected_Sock_Disconected();
				break;
			}
			if (nReceivedBytes < 0) {
				printf("error\n");
				DWORD err = WSAGetLastError();
				break;
			}
			pParent->NetMsg_2_FSMMsg(buffer, nReceivedBytes);

			Sleep(1000); 
			
		} while(1);

	}

	delete [] buffer;
	return 1;
}