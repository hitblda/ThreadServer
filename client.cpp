// ConsoleTCP.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
//
// 
/*
1 ���շ���˵���Ϣ
2 ������Ϣ�������
3 �˳�����
*/

#pragma comment(lib,"ws2_32.lib")  //��Ҫ�����

#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <process.h>

#define NameSize 32
#define BufSize  64

char szName[NameSize] = "[DEFAULT]";
char szMsg[BufSize] = {0};

unsigned WINAPI SendMsg(void *arg)
{
	memset(szMsg, 0, sizeof(szMsg));
	//1 ���մ��ݹ����Ĳ���
	SOCKET hClnSock = *((SOCKET*)arg);

	//2 ѭ���������Կ���̨����Ϣ
	char szSendMsg[NameSize + BufSize];
	while (1)
	{
		fgets(szMsg,BufSize,stdin);// �������������ǣ��ӿ���̨����
		//�����ڸþ䣡����ֱ���յ���������

		//�յ�q����Q���˳�
		if (!strcmp(szMsg, "Q\n") || !strcmp(szMsg, "q\n"))
		{
			closesocket(hClnSock);
			exit(0);
		}

		sprintf(szSendMsg, "%s %s", szName, szMsg);//�ַ�����ƴ�ӣ�����ŵ��µ�����
		send(hClnSock, szSendMsg, strlen(szSendMsg), 0);
	}

	return 0;
}

unsigned WINAPI RecvMsg(void* arg)
{
	memset(szMsg, 0, sizeof(szMsg));

	SOCKET hCLnSock = *((SOCKET*)arg);
	char szRecvMsg[NameSize + BufSize];
	int iLen = 0;
	while (1)
	{
		iLen = recv(hCLnSock, szRecvMsg, NameSize + BufSize - 1, 0);
		//recv��������
		if (iLen==-1)  //�������˶Ͽ����ղ�����������-1
		{
			return -1;
			//�������˶Ͽ�������ֻ��������߳�
			//���͵��߳���ô��ֹ����
			//��ô��ֹ���̣�
		}
		//szRecvMsg��0��iLen-1 ���ǽ��յ�������
		//�����յ������ݷŵ��ն�
		szRecvMsg[iLen] = 0;
		fputs(szRecvMsg, stdout);
	}

	return 0;
}

//��������main��������Ҫ������������ 
//�ڵ�ǰĿ¼�� shift+����Ҽ� cmd
int main(int argc,char *argv[])
{
	memset(szMsg, 0, sizeof(szMsg));

	std::cout << "Hello ,I'm client !\n" << std::endl;
	std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" << std::endl;
	//��ʼ���׽ӿ�
	WORD nVersion;
	WSADATA wsaData;
	int err;
	SOCKET hSock;
	SOCKADDR_IN servAddr;

	HANDLE hSendThread, hRecvThread;
	nVersion = MAKEWORD(1, 1);
	err = WSAStartup(nVersion, &wsaData); //�õ�һ��������ʼ����������浽�ڶ�����������
	if (err != 0)
	{
		return err;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		std::cout << "�汾��ʼ������!\n";
		WSACleanup(); //����
		return -1;
	}
	sprintf(szName,"[%s]",argv[1]);

	//1�����׽���
	hSock = socket(AF_INET, SOCK_STREAM, 0); //����IPV4��TCP

	//2���ö˿ں͵�ַ
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //������IP��ַ 127.0.0.1�Ǳ���LoopBack
	servAddr.sin_port = htons(6000); //����ͷ�������һ��
	servAddr.sin_family = AF_INET;

	//3 ���ӷ�����
	int ret = connect(hSock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	if (ret == SOCKET_ERROR)
	{
		std::cout << "client connects error,error code is" << GetLastError() << std::endl;
		std::cout<<"***************************"<<std::endl;
		std::cout<<ret<<std::endl;
	}

	//4 ��һ���̣߳����շ���������������Ϣ
	hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);

	//5 ��һ���̣߳�������Ϣ��������
	hRecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSock, 0, NULL);

	//�ȴ��ں˶�����źŷ����仯
	WaitForSingleObject(hSendThread, INFINITE);
	WaitForSingleObject(hRecvThread, INFINITE);

	//6 �ر��׽���
	closesocket(hSock);

	//�����׽���
	WSACleanup();

	system("pause");
	return 0;
}
