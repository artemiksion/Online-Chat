#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <conio.h>
#include <string>
#pragma warning(disable: 4996)

std::mutex MT;

struct NodeOfStack
{
	SOCKET Connection;
	NodeOfStack* NextNode;
	char* UserName;
	int SizeOfUserName;
};

NodeOfStack* TopNode = nullptr;

void DeleteThisNode(NodeOfStack* ThisNode)
{
	MT.lock();
	if (TopNode == ThisNode)
	{
		TopNode = ThisNode->NextNode;
		delete ThisNode;
	}
	else
	{
		NodeOfStack* p = TopNode;
		while (true)
		{
			if (p->NextNode == ThisNode)
			{
				p->NextNode = ThisNode->NextNode;
				closesocket(ThisNode->Connection);
				delete[] ThisNode->UserName;
				delete ThisNode;
				break;
			}
			p = p->NextNode;
		}
	}
	MT.unlock();
	throw std::exception();
}

int GetSize(NodeOfStack* ThisNode)
{
	int Size;
	if (recv(ThisNode->Connection, (char*)&Size, sizeof(int), NULL) == -1)
	{
		DeleteThisNode(ThisNode);
	}
	return Size;
}

char* GetString(NodeOfStack* ThisNode, int Size)
{
	char* string = new char[Size];
	if (recv(ThisNode->Connection, string, Size, NULL) == -1)
	{
		DeleteThisNode(ThisNode);
	}
	return string;
}
											
void ClientHandler(NodeOfStack* ThisNode)
{
	try
	{
		ThisNode->SizeOfUserName = GetSize(ThisNode);
		ThisNode->UserName = GetString(ThisNode, ThisNode->SizeOfUserName);
	}
	catch (std::bad_alloc)
	{
		DeleteThisNode(ThisNode);
		return;
	}
	catch (std::exception)
	{
		return;
	}
	char* msg = nullptr;
	int msgSize;
	while (true)
	{
		try
		{
			msgSize = GetSize(ThisNode);
			msg = GetString(ThisNode, msgSize);
		}
		catch (std::bad_alloc)
		{
			DeleteThisNode(ThisNode);
			return;
		}
		catch (std::exception)
		{
			return;
		}
		MT.lock();
		NodeOfStack* p = TopNode;
		while (p != nullptr)
		{
			if (p != ThisNode)
			{
				send(p->Connection, (char*)(&(ThisNode->SizeOfUserName)), sizeof(int), NULL);
				send(p->Connection, ThisNode->UserName, ThisNode->SizeOfUserName, NULL);
				send(p->Connection, (char*)&msgSize, sizeof(int), NULL);
				send(p->Connection, msg, msgSize, NULL);
			}
			p = p->NextNode;
		}
		MT.unlock();
		delete[] msg;
		msg = nullptr;
	}
}

void GetNewConnection(SOCKET& sListen, SOCKADDR_IN& addr)
{
	SOCKET newConnection;
	int SizeOfaddr = sizeof(addr);
	while (true)
	{
		newConnection = accept(sListen, (SOCKADDR*)&addr, &SizeOfaddr);
		if (newConnection == INVALID_SOCKET)
		{
			return;
		}
		else
		{
			try
			{
				NodeOfStack* p = new NodeOfStack;
				p->Connection = newConnection;
				p->UserName = nullptr;
				MT.lock();
				p->NextNode = TopNode;
				TopNode = p;
				MT.unlock();
				std::thread T2(ClientHandler, p);
				T2.detach();
			}
			catch (std::bad_alloc())
			{
				continue;
			}
		}
	}
}

void ShowUsernames()
{
	MT.lock();
	NodeOfStack* p = TopNode;
	while (p != nullptr)
	{
		for (int i = 0; i < p->SizeOfUserName; i++)
		{
			std::cout << p->UserName[i];
		}
		std::cout << std::endl;
		p = p->NextNode;
	}
	MT.unlock();
}

void DisconnectAllUsers()
{
	MT.lock();
	NodeOfStack* p = TopNode;
	while (p != nullptr)
	{
		closesocket(p->Connection);
		p = p->NextNode;
	}
	MT.unlock();
}

void DisconectUserWithName()
{
	std::string Name;
	std::cout << "Input name:" << std::endl;
	std::cin >> Name;
	std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
	MT.lock();
	NodeOfStack* p = TopNode;
	NodeOfStack* prev = TopNode;
	while (p != nullptr)
	{
		int i;
		for (i = 0; i < p->SizeOfUserName; i++)
		{
			if (p->UserName[i] != Name[i])
				break;
		}
		if ((i == p->SizeOfUserName) && (Name.size() == p->SizeOfUserName))
		{
			closesocket(p->Connection);
			break;
		}
		prev = p;
		p = p->NextNode;
	}
	MT.unlock();
}

int main()
{
	WSADATA wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0)
	{
		std::cout << "Library initialization error" << std::endl;
		system("pause");
		return 1;
	}
	SOCKADDR_IN addr;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);
	std::thread TT(GetNewConnection, std::ref(sListen), std::ref(addr));
	TT.detach();
	while (true)
	{
		std::cout << "Choose action with server:" << std::endl;
		std::cout << "1. Show usernames" << std::endl;
		std::cout << "2. Disconnect all users" << std::endl;
		std::cout << "3. Disconnect user with name..." << std::endl;
		std::cout << "4. Close server" << std::endl;
		int c = _getch() - 48;
		switch (c)
		{
		case 1:
			ShowUsernames();
			system("pause");
			break;
		case 2:
			DisconnectAllUsers();
			break;
		case 3:
			DisconectUserWithName();
			break;
		case 4:
			DisconnectAllUsers();
			closesocket(sListen);
			WSACleanup();
			return 0;
		default:
			break;
		}
		system("cls");
	}
	return 0;
}