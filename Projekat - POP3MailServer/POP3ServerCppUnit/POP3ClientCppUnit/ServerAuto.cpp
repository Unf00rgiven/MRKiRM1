#include <stdio.h>

#include "const.h"
#include "ServerAuto.h"
#include "dirent.h"
#include <thread.h>

bool g_ProgramEnd = false;

#define StandardMessageCoding 0x00

char* users[4] = { "Ognjen\0", "Bozidar\0", "Anastasija\0", "Milos\0" };
char* passwords[4] = { "gio123\0", "bokibo13\0", "anja12345\0", "12345\0" };
int user_index;


/*
*params:
*3. number of time control
*4. max states for one automate
*5. max number of transitions functions
*/
SrAuto::SrAuto() : FiniteStateMachine(SR_AUTOMATE_TYPE_ID, SR_AUTOMATE_MBX_ID, 0, FSM_SR_STATE_COUTN, 3) {
}

SrAuto::~SrAuto() {
}

/* This function actually connnects the ClAutoe with the mailbox. */
uint8 SrAuto::GetMbxId() {
	return SR_AUTOMATE_MBX_ID;
}

uint32 SrAuto::GetObject() {
	return GetObjectId();
}

MessageInterface *SrAuto::GetMessageInterface(uint32 id) {
	return &StandardMsgCoding;
}

void SrAuto::SetDefaultHeader(uint8 infoCoding) {
	SetMsgInfoCoding(infoCoding);
	SetMessageFromData();
}

void SrAuto::SetDefaultFSMData() {
	SetDefaultHeader(StandardMessageCoding);
}

void SrAuto::NoFreeInstances() {
	printf("[%d] SrAuto::NoFreeInstances()\n", GetObjectId());
}

void SrAuto::Reset() {
	printf("[%d] SrAuto::Reset()\n", GetObjectId());
}


void SrAuto::Initialize() {
	SetState(FSM_SR_IDLE);

	//intitialization message handlers
	InitEventProc(FSM_SR_IDLE, MSG_Conn_req, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Idle_Set_All);

	//Username login
	InitEventProc(FSM_SR_AUTHORISING_USERNAME, MSG_user, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Authorising_username);
	InitEventProc(FSM_SR_AUTHORISING_USERNAME, MSG_quit, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Disconnect);
	InitEventProc(FSM_SR_AUTHORISING_USERNAME, MSG_other, (PROC_FUN_PTR)&SrAuto::FSM_SR_Invalid_Request);

	//Password login
	InitEventProc(FSM_SR_AUTHORISING_PASSWORD, MSG_pass, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Authorising_pass);
	InitEventProc(FSM_SR_AUTHORISING_PASSWORD, MSG_quit, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Disconnect);
	InitEventProc(FSM_SR_AUTHORISING_PASSWORD, MSG_other, (PROC_FUN_PTR)&SrAuto::FSM_SR_Invalid_Request);

	//Transaction
	InitEventProc(FSM_SR_TRANSACTION, MSG_stat, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Mail_Check);
	InitEventProc(FSM_SR_TRANSACTION, MSG_quit, (PROC_FUN_PTR)&SrAuto::FSM_Sr_Disconnect);
	InitEventProc(FSM_SR_TRANSACTION, MSG_other, (PROC_FUN_PTR)&SrAuto::FSM_SR_Invalid_Request);
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

void SrAuto::FSM_Sr_Idle_Set_All() {
	int c;

	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return;
	}

	//Create socket
	m_Server_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_Server_Socket == -1)
	{
		printf("TCP : Could not create socket");
	}
	puts("TCP : Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(m_Server_Socket, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		//print the error message
		perror("TCP : bind failed. Error");
		return;
	}
	puts("TCP : bind done");

	//Listen
	listen(m_Server_Socket, 3);

	//Accept and incoming connection
	puts("TCP : Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	//accept connection from an incoming client
	m_Client_Socket = accept(m_Server_Socket, (struct sockaddr *)&client, (int*)&c);
	if (m_Client_Socket < 0)
	{
		perror("TCP : accept failed");
		return;
	}
	puts("TCP : Client connected");


	char message[] = "+OK\r\n";
	VignerEncryption(message);

	if (send(m_Client_Socket, message, strlen(message), 0) < 0)
	{
		puts("Send failed");
		return;
	}


	SetState(FSM_SR_AUTHORISING_USERNAME);

	/* Then, start the thread that will listen on the the newly created socket. */
	m_hThread = CreateThread(NULL, 0, ServerListener, (LPVOID) this, 0, &m_nThreadID);
	if (m_hThread == NULL) {
		/* Cannot create thread.*/
		closesocket(m_Server_Socket);
		m_Server_Socket = INVALID_SOCKET;
		return;
	}
}

void SrAuto::FSM_Sr_Authorising_username() {
	int user_valid = 0;

	char* data = new char[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];
	memcpy(data, buffer + 4, size);
	data[size - 2] = 0;
	VignerDencryption(data + 5);
	printf("%s\n", data);

	for (int i = 0; i < NUM_USERS; i++)
	{
		if (strcmp(data + 5, users[i]) == 0) //provera user-a
		{
			user_valid = 1;
			user_index = i;
			break;
		}
	}

	char message[50];
	if (user_valid == 1)
	{
		//Send some data
		strcpy(message, "+OK\r\n\0");
		VignerEncryption(message);
		if (send(m_Client_Socket, message, strlen(message), 0) < 0)
		{
			puts("Send failed");
			return;
		}

		SetState(FSM_SR_AUTHORISING_PASSWORD);

	}
	else {
		strcpy(message, "-ERROR Wrong user\r\n\0");
		VignerEncryption(message);
		if (send(m_Client_Socket, message, strlen(message), 0) < 0)
		{
			puts("Send failed");
			return;
		}

		SetState(FSM_SR_AUTHORISING_USERNAME);
	}
	delete[] data;
}

void SrAuto::FSM_Sr_Authorising_pass() {
	int pass_valid = 0;

	char* data = new char[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data, buffer + 4, size);
	data[size - 2] = 0;
	VignerDencryption(data + 5);
	printf("%s\n", data);

	if (strcmp(data + 5, passwords[user_index]) == 0) //provera user-a
	{
		pass_valid = 1;
	}

	char message[20];
	if (pass_valid == 1)
	{
		//Send some data
		strcpy(message, "+OK\r\n");
		VignerEncryption(message);
		if (send(m_Client_Socket, message, strlen(message), 0) < 0)
		{
			puts("Send failed");
			return;
		}

		SetState(FSM_SR_TRANSACTION);

	}
	else {
		strcpy(message, "-ERROR Wrong password\r\n\0");
		VignerEncryption(message);
		if (send(m_Client_Socket, message, strlen(message), 0) < 0)
		{
			puts("Send failed");
			return;
		}

		SetState(FSM_SR_AUTHORISING_PASSWORD);
	}
	delete[] data;
}

void SrAuto::FSM_Sr_Mail_Check() {

	char* data = new char[255];
	uint8* buffer = GetParam(PARAM_DATA);
	uint16 size = buffer[2];

	memcpy(data, buffer + 4, size);
	data[size - 2] = 0;
	VignerDencryption(data + 5);
	printf("%s\n", data);

	char message[20];
	strcpy(message, "+OK 0 0\r\n");
	VignerEncryption(message);
	if (send(m_Client_Socket, message, strlen(message), 0) < 0)
	{

		return;
	}

	SetState(FSM_SR_TRANSACTION);
	delete[] data;
	
}

void SrAuto::FSM_SR_Invalid_Request()
{
	char message[50];
	strcpy(message, "-ERR 404 Invalid request \r\n");
	VignerEncryption(message);
	puts("Waiting for another request");
	if (send(m_Client_Socket, message, strlen(message), 0) < 0)
	{
		puts("Send failed");
		return;
	}
}

void SrAuto::FSM_Sr_Disconnect() {

	char message[50];

	strcpy(message, "+OK\r\n");
	VignerEncryption(message);
	if (send(m_Client_Socket, message, strlen(message), 0) < 0)
	{
		return;
	}

	shutdown(m_Client_Socket, 2);
	shutdown(m_Server_Socket, 2);

	CloseHandle(m_hThread);
	closesocket(m_Client_Socket);
	closesocket(m_Server_Socket);

	g_ProgramEnd = true;

	SetState(FSM_SR_IDLE);
}

/* This metdod sendig message to activate current state */
void SrAuto::NetMsg_2_FSMMsg(const char* apBuffer, uint16 anBufferLength) {
	// TODO: PROMENITI MSG_MSG U SWITCH CASE
	int i = 0;
	char* operationBuffer = new char[16];
	for (int i = 0; i < 4; i++)
	{
		operationBuffer[i] = apBuffer[i];
	}
	if (!strncmp(operationBuffer, "user", 4))
	{
		PrepareNewMessage(0x00, MSG_user);
	}
	else if (!strncmp(operationBuffer, "pass", 4))
	{
		PrepareNewMessage(0x00, MSG_pass);
	}
	else if (!strncmp(operationBuffer, "stat", 4))
	{
		PrepareNewMessage(0x00, MSG_stat);
	}
	else if (!strncmp(operationBuffer, "retr", 4))
	{
		PrepareNewMessage(0x00, MSG_retr);
	}
	else if (!strncmp(operationBuffer, "quit", 4))
	{
		PrepareNewMessage(0x00, MSG_quit);
	}
	else 
	{
		PrepareNewMessage(0x00, MSG_other);
	}

	SetMsgToAutomate(SR_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	AddParam(PARAM_DATA, anBufferLength, (uint8 *)apBuffer);
	SendMessage(SR_AUTOMATE_MBX_ID);
	delete[] operationBuffer;

}

DWORD SrAuto::ServerListener(LPVOID param) {
	SrAuto* pParent = (SrAuto*)param;
	int nReceivedBytes;
	char* buffer = new char[255];


	/* Receive data from the network until the socket is closed. */
	do {
		nReceivedBytes = recv(pParent->m_Client_Socket, buffer, 255, 0);
		if (nReceivedBytes == 0)
		{
			printf("Client disconnected!\n");
			pParent->FSM_Sr_Disconnect();
			break;
		}
		if (nReceivedBytes < 0) {
			//printf("Failed with error: %d\n", WSAGetLastError());
			printf("Client disconnected!\n");
			pParent->FSM_Sr_Disconnect();
			break;
		}
		pParent->NetMsg_2_FSMMsg(buffer, nReceivedBytes);

		Sleep(1000);

	} while (1);



	delete[] buffer;
	return 1;
}

/* Automat sending message to yourself for starting system */
void SrAuto::Start() {

	PrepareNewMessage(0x00, MSG_Conn_req);
	SetMsgToAutomate(SR_AUTOMATE_TYPE_ID);
	SetMsgObjectNumberTo(0);
	SendMessage(SR_AUTOMATE_MBX_ID);
}