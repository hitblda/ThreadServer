// ConsoleTCP.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
//
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#pragma comment(lib,"ws2_32.lib")  //��Ҫ�����

#include <WinSock2.h>
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <process.h>

#define MAX_CLNT 256
#define MAX_MSG  256

SOCKET clnSocket[MAX_CLNT];
int clnCnt = 0; //�Ѿ����ӵ���

HANDLE hMutex;

void SendMsg(char* szMsg, int len)
{
	int i = 0;
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < clnCnt; i++)
	{
		//���͸����еĿͻ���
		send(clnSocket[i], szMsg, len, 0);
	}
	ReleaseMutex(hMutex);
}

//����ͻ������ӵĺ���
unsigned WINAPI handleClient(void* arg)
{
	//Step1 ���ղ���
	SOCKET hClntSock = *((SOCKET*)arg);

	//Step2 �����շ�
	char recvBuf[MAX_MSG];
	char sendBuf[MAX_MSG];


	sprintf(sendBuf, "Hello,I'm srv,who are u.\n");

	//���Ӳ��Ͽ�ʱ���շ����Ͻ���
	while (1)
	{ 
		memset(recvBuf, 0, sizeof(recvBuf));
		memset(sendBuf, 0, sizeof(sendBuf));

		int iLen = recv(hClntSock, recvBuf, sizeof(recvBuf), 0);
		////����ʽ�ģ������ݲŷ��� 
		////���յ����� ���͸����еĿͻ���

		//Step3 �ͻ��˶Ͽ�����
		//���յ������ݳ���Ϊ-1��ʾ���Ӹö˿ڵĿͻ����Ѿ��˳�
		//��Ҫ����������1��������Ҫע�ⲻҪԽ�磬����0Ϊֹ
		if (iLen == -1 && clnCnt >0)  
		{
			WaitForSingleObject(hMutex, INFINITE);
			for (int i = 0; i < clnCnt; i++)
			{
				if (hClntSock == clnSocket[i])
				{
					//��λ
					while (i < clnCnt)
					{
						clnSocket[i] = clnSocket[i + 1];
						i++;
					}
					break;
					//�ҵ������ж�������ѭ��
				}
			}
			clnCnt--;
			ReleaseMutex(hMutex);
		}
		if (clnCnt==0)
		{
			printf("��ʱû�пͻ������ӣ�\n");
			return -1;
		}
		else
		{
			printf("��ʱ��������Ŀ�� %d\n", clnCnt);
			printf("recv length is %d\n", iLen);
			printf("recv message is %s\n", recvBuf);

			SendMsg(recvBuf, sizeof(recvBuf));
		}

		//
		////std::cout << recvBuf << std::endl;
	}

// WaitForSingleObject(hMutex, INFINITE);
// clnCnt--;
// ReleaseMutex(hMutex);
	closesocket(hClntSock);
	return 0;
}

int main()
{
	std::cout << "Hello ,I'm server !\n" << std::endl;
	std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" << std::endl;
	//��ʼ���׽ӿ�
	WORD nVersion;
	WSADATA wsaData;
	int err;

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

	//����һ���������
	hMutex = CreateMutex(NULL, FALSE, NULL);

	//�����׽���
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0); //����IPV4��TCP

	//׼���󶨵���Ϣ
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//�󶨵��������κ���������
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(6000); //�˿ںţ�0~65535
	//ÿ���˿ںŶ�Ӧһ������ע�⣺1024���µ�Ϊ����ϵͳԤ��ʹ�ã�

	//���׽���
	if (bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		std::cout << "server bind error,error num is " << GetLastError<< std::endl;
		return -1;
	}

	//����
	if(listen(sockSrv, 10)==SOCKET_ERROR) //���׽��֣�ͬʱ��������
	{
		std::cout << "server listen error,error num is " << GetLastError << std::endl;
		return -1;
	}
	printf("Start listen .......................................\n");

	std::cout << "server start at port 6000" << std::endl;
	SOCKADDR_IN addrCli; //������SOCKADDR_IN�����Ա����
	int len = sizeof(SOCKADDR); //����Ϊʲô��SOCKADDR����

	while (true)
	{
		//�����������󣬷�����Կͻ��˵��׽���
		SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrCli, &len);
		//���յ���������Ͱѿͻ��˵�ַ�ŵ��ڶ�����������
		//����ú������ڶ���������û�гɹ����ӣ�
		//������˷��ؾ������ӳɹ�������ֵ�������ӵ��׽���

		WaitForSingleObject(hMutex, INFINITE);
		//ÿ��һ�����ӣ�ȫ����������һ����Ա�������������1
		clnSocket[clnCnt++] = sockConn;
		ReleaseMutex(hMutex);

		//ÿ��һ�����ӣ��½�һ���̣߳�ά�ֺͷ����������ӹ�ϵ
		HANDLE bThread;
		bThread = (HANDLE)_beginthreadex(NULL,0, handleClient,(void*)&sockConn,0,NULL);

		printf("���µĿͻ����������ˣ�\n");
		printf("client IP address is %s\n", inet_ntoa(addrCli.sin_addr));
		std::cout<<"client num is"<< clnCnt<<std::endl; //���ӵ���Ŀ
	}

	//�ر��׽���
	closesocket(sockSrv);

	//�����׽���
	WSACleanup();

	system("pause");
	return 0;
}

