#include <stdio.h>
#include <string.h>
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "RakNetTypes.h"
#include "RakPeerInterface.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 60000

enum GameMessages {
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1
};

int main(void)
{
	char str[512];

	RakNet::RakPeerInterface *peer = RakNet::RakPeerInterface::GetInstance();
	bool isServer;
	RakNet::Packet *packet;

	printf("(C) or (S)server?\n");
	gets(str);

	if ((str[0] == 'c') || str[0] == 'C')
	{
		RakNet::SocketDescriptor sd;
		peer->Startup(1, &sd, 1);
		isServer = false;
	}
	else {
		RakNet::SocketDescriptor sd(SERVER_PORT, 0);
		peer->Startup(MAX_CLIENTS, &sd, 1);
		isServer = true;
	}

	if (isServer)
	{
		printf("Starting the server.\n");
		// Let the server accept incoming connections
		peer->SetMaximumIncomingConnections(MAX_CLIENTS);
	}
	else
	{
		printf("Enter server IP or hit enter for localhost\n");
		gets(str);
		if (str[0] == 0) {
			strcpy(str, "127.0.0.1");
		}
		printf("Starting the client.\n");
		peer->Connect(str, SERVER_PORT, 0, 0);
	}

	while (1)
	{
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive()){
			switch (packet->data[0]) 
			{
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				printf("Another client has disconnected.\n");
				break;
			case ID_REMOTE_CONNECTION_LOST:
				printf("Another client has lost the connection.\n");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				printf("Another client has connected.\n");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				printf("Our connection request has been accepted.\n");

				// Use Bitstream to write message
				// Bitstreams are easier to use than sending casted structured, and handle endian issues automatically
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
				bsOut.Write("Hello world!");
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}
				break;
			case ID_NEW_INCOMING_CONNECTION:
				printf("A connection is incoming...\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				printf("The server is full.\n");
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				if (isServer) {
					printf("A client has disconnected.\n");
				}
				else {
					printf("We have been disconnected from the server.\n");
				}
				break;
			case ID_CONNECTION_LOST:
				if (isServer) {
					printf("A client lost the connection\n");
				}
				else {
					printf("Connection to server lost.\n");
				}
				break;
			case ID_GAME_MESSAGE_1:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
			}
				break;
			default:
				printf("Message with identifier %i has arrived\n", packet->data[0]);
				break;
			}
		}
	}

	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}