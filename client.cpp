// ConsoleTCP.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
// 
/*
1 接收服务端的消息
2 发送消息给服务端
3 退出机制
*/

#pragma comment(lib,"ws2_32.lib")  //需要网络库

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
	//1 接收传递过来的参数
	SOCKET hClnSock = *((SOCKET*)arg);

	//2 循环接收来自控制台的消息
	char szSendMsg[NameSize + BufSize];
	while (1)
	{
		fgets(szMsg,BufSize,stdin);// 第三个参数就是，从控制台接收
		//阻塞在该句！！！直到收到东西！！

		//收到q或者Q就退出
		if (!strcmp(szMsg, "Q\n") || !strcmp(szMsg, "q\n"))
		{
			closesocket(hClnSock);
			exit(0);
		}

		sprintf(szSendMsg, "%s %s", szName, szMsg);//字符串的拼接，结果放到新的数组
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
		//recv是阻塞的
		if (iLen==-1)  //服务器端断开，收不到东西就是-1
		{
			return -1;
			//服务器端断开，这样只是提出本线程
			//发送的线程怎么终止？？
			//怎么终止进程？
		}
		//szRecvMsg的0到iLen-1 都是接收到的数据
		//将接收到的数据放到终端
		szRecvMsg[iLen] = 0;
		fputs(szRecvMsg, stdout);
	}

	return 0;
}

//带参数的main函数，需要以命令行启动 
//在当前目录下 shift+鼠标右键 cmd
int main(int argc,char *argv[])
{
	memset(szMsg, 0, sizeof(szMsg));

	std::cout << "Hello ,I'm client !\n" << std::endl;
	std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" << std::endl;
	//初始化套接库
	WORD nVersion;
	WSADATA wsaData;
	int err;
	SOCKET hSock;
	SOCKADDR_IN servAddr;

	HANDLE hSendThread, hRecvThread;
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
	sprintf(szName,"[%s]",argv[1]);

	//1创建套接字
	hSock = socket(AF_INET, SOCK_STREAM, 0); //基于IPV4的TCP

	//2配置端口和地址
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //服务器IP地址 127.0.0.1是本机LoopBack
	servAddr.sin_port = htons(6000); //必须和服务器端一样
	servAddr.sin_family = AF_INET;

	//3 连接服务器
	int ret = connect(hSock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	if (ret == SOCKET_ERROR)
	{
		std::cout << "client connects error,error code is" << GetLastError() << std::endl;
		std::cout<<"***************************"<<std::endl;
		std::cout<<ret<<std::endl;
	}

	//4 起一个线程，接收服务器发送来的消息
	hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);

	//5 起一个线程，发送消息给服务器
	hRecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSock, 0, NULL);

	//等待内核对象的信号发生变化
	WaitForSingleObject(hSendThread, INFINITE);
	WaitForSingleObject(hRecvThread, INFINITE);

	//6 关闭套接字
	closesocket(hSock);

	//清理套接字
	WSACleanup();

	system("pause");
	return 0;
}
