#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MaxSize 4096
#pragma once
#include<iostream>
#include<fstream>
#include<windows.h>
#include<time.h>
#include<string>
#include<stdlib.h>
#include<vector>
#include"SOCKET.h"
#include<WinSock2.h>
#include<algorithm>
#define information "DNS_information.txt"

using namespace std;
#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")

HANDLE hMutex;
int client_number = 0;
int subjective_choice = 0;

struct IP_TRANS
{
	string ip;
	string net;
};

void Begin(SOCKET _cSock)
{
	cout << "已经连接一个客户端" << endl;
	WaitForSingleObject(hMutex, INFINITE);
	send_(_cSock, "Please inpout the net you want to trans to IP address:\n");
	send_(_cSock, "#");
	char NetName[MaxSize];//保存用户输入的网址
	int temp_recv_len = 0;
	temp_recv_len = recv(_cSock, NetName, MaxSize, 0);
	NetName[temp_recv_len] = '\0';//结尾补充结束符，保存网址完毕

	ifstream fin(information);
	vector<IP_TRANS> data;
	IP_TRANS temp;
	while (fin >> temp.ip)
	{
		fin >> temp.net;
		data.push_back(temp);
		temp.ip.clear();
		temp.net.clear();
	} //从文件中读取已经存在的转换表
	
	while (strcmp(NetName,"#")!=0)//用户输入#退出查询
	{
		int i = 0;
		for (;i < data.size()&&data[i].net != NetName;i++);

		if (i >= data.size())//IP在本地不存在，需要向上一级请求
		{
			send_(_cSock, "this IP does not exist\n");
			
		}

		else if (data[i].ip == "0.0.0.0")//IP为广播地址
		{
			send_(_cSock, "This IP is a broadcast and it may has risk\n");
			
		}
		else
		{
			char *ip_char = (char*)data[i].ip.data();
			send_(_cSock, ip_char);
			send_(_cSock, "\n");
			
		}
		send_(_cSock, "Please input the next net you want to search:\n");
		send_(_cSock, "#");
		temp_recv_len = recv(_cSock, NetName, MaxSize, 0);
		NetName[temp_recv_len] = '\0';

	}
	send_(_cSock, "###");
	fin.close();
	client_number--;
	cout << "当前连接的客户端总数为：" << client_number << endl;
	cout << "ONE CLIENT QUIT!" << endl;
}


int main()
{
	SOCKADDR_IN _sin;
	SOCKET_CREATE();

	//1.创立套接字socket

	_sin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(6666);

	int _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == _sock)
		cout << "socket Create ERROR" << endl;
	//2.bind绑定客户端

	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
		cout << "ERROR,CONNETING Failed..." << endl;
	else
		cout << "Succeed!..." << endl;

	//listen监听
	if (SOCKET_ERROR == listen(_sock, 5))//五个监听端口
		cout << "ERROR! Listening failed..." << endl;
	else
		cout << "Succeed!..." << endl;

	//4.等待客户端连接成功accept
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;

	while (1)
	{
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);

		if (_cSock == INVALID_SOCKET)
			cout << "invalid client..." << endl;
		else
			cout << "Succeed!" << endl;

		//5.向客户端发送消息
		HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Begin, (LPVOID)_cSock, 0, 0);

		if (client_number == 0)
		{
			cout << "是否选择关闭主机：Y/N" << endl;
			char temp;
			cin >> temp;
			if (temp == 'Y')
				break;
		}

		if (hThread != NULL)
		{
			CloseHandle(hThread);
			cout << "connecting with a new client!" << endl;
			client_number++;
		}


		Sleep(100);
	}

	cout << "脱离连接" << endl;

	//6.关闭套接字
	closesocket(_sock);
	WSACleanup();
	//system("pause");
	return 0;
}
