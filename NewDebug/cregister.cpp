#include "cregister.h"

CRegister::CRegister()
{

}

BOOL CRegister::GetRegisters(DEBUG_EVENT &Event,  CONTEXT &context)
{
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, Event.dwThreadId);
    if(nullptr == hThread)
    {
        return FALSE;
    }

    context.ContextFlags = CONTEXT_FULL;
    if(!GetThreadContext(hThread, &context))
    {
        return FALSE;
    }

    CloseHandle(hThread);

    return TRUE;
}
