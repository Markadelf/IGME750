// NetworkingPlayground.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Serializer.h>
#include <TimeInstableTransform.h>
#include "ServerManager.h"
#include "Scenegraph.h"
#include "ClientHelper.h"
#include "GameHost.h"
#include "StaticObject.h"

void ClientCallback(Buffer& data, int clientId);
void ActiveCallback(Buffer& data, int activeId);
void ReadBuffer(Buffer& bitBuffer);
void ReadBuffer(Address ad, const void* data, const int size);

ServerManager* serverPointer;
GameHost* gameHost;

int main()
{
	int port = 30000;
	ServerManager server(port, 12, 12);
	serverPointer = &server;
	Address serverAddress(127, 0, 0, 1, port);
	server.SetClientCallBack(ClientCallback);
	server.SetActiveCallBack(ActiveCallback);

    gameHost = new GameHost(&server);
	
	while (true)
	{
		server.Listen();
		gameHost->ValidateGameState();
	}
}

void ClientCallback(Buffer& bitBuffer, int clientId) {
	gameHost->HostRecieveClient(bitBuffer, clientId);
}

void ActiveCallback(Buffer& bitBuffer, int playerId) {
	gameHost->HostRecievePlayer(bitBuffer, playerId);
}

void ReadBuffer(Buffer& bitBuffer) {
	char output[21];

	if (Serializer::DeserializeString<30>(bitBuffer, output)) {
		std::cout << output << std::endl;
	}
}

void ReadBuffer(Address ad, const void* data, const int size)
{
	Buffer buff(size);
	buff.WriteBytes(data, size);
	buff.ResetIndex();
	ReadBuffer(buff);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
