// ConsoleTCP.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#pragma comment(lib,"ws2_32.lib")  //需要网络库

#include <WinSock2.h>
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <process.h>

#define MAX_CLNT 256
#define MAX_MSG  256

SOCKET clnSocket[MAX_CLNT];
int clnCnt = 0; //已经连接的数

HANDLE hMutex;

void SendMsg(char* szMsg, int len)
{
	int i = 0;
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < clnCnt; i++)
	{
		//发送给所有的客户端
		send(clnSocket[i], szMsg, len, 0);
	}
	ReleaseMutex(hMutex);
}

//处理客户端连接的函数
unsigned WINAPI handleClient(void* arg)
{
	//Step1 接收参数
	SOCKET hClntSock = *((SOCKET*)arg);

	//Step2 进行收发
	char recvBuf[MAX_MSG];
	char sendBuf[MAX_MSG];


	sprintf(sendBuf, "Hello,I'm srv,who are u.\n");

	//连接不断开时，收发不断进行
	while (1)
	{ 
		memset(recvBuf, 0, sizeof(recvBuf));
		memset(sendBuf, 0, sizeof(sendBuf));

		int iLen = recv(hClntSock, recvBuf, sizeof(recvBuf), 0);
		////阻塞式的，有数据才返回 
		////有收到数据 发送给所有的客户端

		//Step3 客户端断开处理
		//接收到的数据长度为-1表示连接该端口的客户端已经退出
		//需要将连接数减1，另外需要注意不要越界，减到0为止
		if (iLen == -1 && clnCnt >0)  
		{
			WaitForSingleObject(hMutex, INFINITE);
			for (int i = 0; i < clnCnt; i++)
			{
				if (hClntSock == clnSocket[i])
				{
					//移位
					while (i < clnCnt)
					{
						clnSocket[i] = clnSocket[i + 1];
						i++;
					}
					break;
					//找到，就中断最外层的循环
				}
			}
			clnCnt--;
			ReleaseMutex(hMutex);
		}
		if (clnCnt==0)
		{
			printf("此时没有客户端连接！\n");
			return -1;
		}
		else
		{
			printf("此时的连接数目是 %d\n", clnCnt);
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
	//初始化套接库
	WORD nVersion;
	WSADATA wsaData;
	int err;

	nVersion = MAKEWORD(1, 1);
	err = WSAStartup(nVersion, &wsaData); //用第一个参数初始化，结果保存到第二个参数里面
	if (err != 0)
	{
		return err;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		std::cout << "版本初始化错误!\n";
		WSACleanup(); //清理
		return -1;
	}

	//创建一个互斥对象
	hMutex = CreateMutex(NULL, FALSE, NULL);

	//创建套接字
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0); //基于IPV4的TCP

	//准备绑定的信息
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//绑定到本机的任何网卡上面
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(6000); //端口号：0~65535
	//每个端口号对应一个服务，注意：1024以下的为操作系统预留使用！

	//绑定套接字
	if (bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		std::cout << "server bind error,error num is " << GetLastError<< std::endl;
		return -1;
	}

	//监听
	if(listen(sockSrv, 10)==SOCKET_ERROR) //本套接字，同时监听多少
	{
		std::cout << "server listen error,error num is " << GetLastError << std::endl;
		return -1;
	}
	printf("Start listen .......................................\n");

	std::cout << "server start at port 6000" << std::endl;
	SOCKADDR_IN addrCli; //必须是SOCKADDR_IN否则成员不对
	int len = sizeof(SOCKADDR); //这里为什么是SOCKADDR？？

	while (true)
	{
		//接收连接请求，返回针对客户端的套接字
		SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrCli, &len);
		//接收到链接请求就把客户端地址放到第二个参数里面
		//如果该函数处于堵塞，就是没有成功连接；
		//如果有了返回就是连接成功，返回值就是连接的套接字

		WaitForSingleObject(hMutex, INFINITE);
		//每来一个连接，全局数组增加一个成员，最大连接数加1
		clnSocket[clnCnt++] = sockConn;
		ReleaseMutex(hMutex);

		//每来一个连接，新建一个线程，维持和服务器的连接关系
		HANDLE bThread;
		bThread = (HANDLE)_beginthreadex(NULL,0, handleClient,(void*)&sockConn,0,NULL);

		printf("有新的客户端来连接了！\n");
		printf("client IP address is %s\n", inet_ntoa(addrCli.sin_addr));
		std::cout<<"client num is"<< clnCnt<<std::endl; //连接的数目
	}

	//关闭套接字
	closesocket(sockSrv);

	//清理套接字
	WSACleanup();

	system("pause");
	return 0;
}

