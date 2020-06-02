#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <WinSock2.h>
#pragma warning(disable: 4996)
#include <thread>
#include <string>
#include <conio.h>

void ClientHandler(SOCKET& connection)
{
	int msgSize;
	char* string;
	char* Name;
	while (true)
	{
		try
		{
			if (recv(connection, (char*)&msgSize, sizeof(int), NULL) == -1)
			{
				break;
			}
			Name = new char[msgSize + 1];
			if (recv(connection, Name, msgSize, NULL) == -1)
			{
				delete[] Name;
				break;
			}
			Name[msgSize] = '\0';
			std::cout << Name << std::endl;
			delete[] Name;
			if (recv(connection, (char*)&msgSize, sizeof(int), NULL) == -1)
			{
				break;
			}
			string = new char[msgSize + 1];
			if (recv(connection, string, msgSize, NULL) == -1)
			{
				delete[] string;
				break;
			}
			string[msgSize] = '\0';
			std::cout << std::endl << string << std::endl << std::endl;
			delete[] string;
		}
		catch (std::bad_alloc)
		{
			std::cout << "Memory error" << std::endl;
		}
	}
	std::cout << "Error of connection with server" << std::endl;
	std::cout << "Input something to end program" << std::endl;
}

int main()
{
	WSADATA wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0)
	{
		std::cout << "Error of library\n";
		_getch();
		return 1;
	}
	SOCKADDR_IN addr;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;
	SOCKET connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(connection, (SOCKADDR*)&addr, sizeof(addr)))
	{
		std::cout << "Connection failed\n";
		_getch();
		return 1;
	}
	std::cout << "Connection established\n";
	std::cout << "Input EXIT to close program" << std::endl;
	std::cout << "Input your nickname(nickname can't start with ' ' and have no symbols, max size of name = 20): " << std::endl;
	std::string Username;
	while (true)
	{
		std::cin >> Username;
		if ((Username[0] == '\n') || (Username[0] == ' ') || (Username.size() > 20))
		{
			std::cout << std::endl << "Pls, repeat input" << std::endl;
			Username.clear();
		}
		else
			break;
	}
	int SizeOfName = Username.size();
	send(connection, (char*)&SizeOfName, sizeof(int), NULL);
	send(connection, Username.c_str(), SizeOfName, NULL);
	Username.clear();
	std::thread T(ClientHandler, std::ref(connection));
	T.detach();
	std::string msg1;
	std::cin.ignore((std::numeric_limits<std::streamsize>::max)(),'\n');
	while (true)
	{
		getline(std::cin, msg1);
		//std::cin >> msg1;
		int msgSize = msg1.size();
		if ((msg1[0] == 'E') && (msg1[1] == 'X') && (msg1[2] == 'I') && (msg1[3] == 'T') && (msgSize == 4))
			break;
		if (send(connection, (char*)&msgSize, sizeof(int), NULL) == SOCKET_ERROR)
			break;
		if (send(connection, msg1.c_str(), msgSize, NULL) == SOCKET_ERROR)
			break;
		Sleep(10);
	}
	closesocket(connection);
	WSACleanup();
	return 0;
}