#include "cmemory.h"

CMemory::CMemory()
{

}

BOOL CMemory::GetTargetMemory(HANDLE hProcess, DWORD dwThreadId, unsigned int address, unsigned int length)
{
    if(nullptr == hProcess)
    {
        return FALSE;
    }

    if(0 == address)
    {
        HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, dwThreadId);
        if(nullptr == hThread)
        {
            OutputDebugStringA("CMemory->GetTargetMemory->OpenThread Error!");
            OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        }

        CONTEXT context;
        context.ContextFlags = CONTEXT_FULL;
        GetThreadContext(hThread, &context);

        CloseHandle(hThread);

        address = context.Eip;
    }

    QByteArray data;
    data.resize(1024);
    SIZE_T bytesRead;
    if(!ReadProcessMemory(hProcess, (LPVOID)address, data.data(),length,&bytesRead))
    {
        OutputDebugStringA("CMemory->GetTargetMemory->OpenThread Error!");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        return FALSE;
    }

    sigMemInfo(data , length, address);

    return TRUE;
}
