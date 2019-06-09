#include "cbreakpoint.h"

CBreakPoint::CBreakPoint()
{
    memset(m_HardBpAry,0,sizeof (m_HardBpAry));
}


BOOL CBreakPoint::SetBreakPoint(HANDLE hProcess, DWORD address, bool frequency)
{
    unsigned char Readdata[1] = {0};
    SIZE_T bytesRead;
    if(!ReadProcessMemory(hProcess, (LPVOID)address, Readdata,1,&bytesRead))
    {
        OutputDebugStringA("ReadProcessMemory Error");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        return FALSE;
    }

    // 保存原来的字节
    BREAK_POINT bpPoint;
    bpPoint.address = address;
    bpPoint.content = Readdata[0];

    char Writedata[1] = { (char)0xcc };
    if(!WriteProcessMemory(hProcess,(LPVOID)address,Writedata,1,&bytesRead))
    {
        OutputDebugStringA("WriteProcessMemory Error");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        return FALSE;
    }

    bpPoint.frequency = frequency;
    // 如果表中没有 就加入到断点表
    if(m_pBpMap->end() == m_pBpMap->find(address))
    {
        m_pBpMap->insert(address,bpPoint);
    }

    return TRUE;
}

BOOL CBreakPoint::RestoreBeakPoint(HANDLE hProcess, DWORD address)
{

    if(m_pBpMap->end() == m_pBpMap->find(address))
    {
        OutputDebugStringA("m_pBpMap not find break point!");
        return FALSE;
    }
    unsigned char data[1] = {0};
    data[0] = m_pBpMap->find(address).value().content;

    SIZE_T bytesRead = 0;
    if(!WriteProcessMemory(hProcess,(LPVOID)address,data,1,&bytesRead))
    {
        OutputDebugStringA("WriteProcessMemory Error!");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        return FALSE;
    }

    return TRUE;
}

BOOL CBreakPoint::SetMemBreakPoint(HANDLE hProcess, DWORD address,SIZE_T nSize)
{
    // 如果长度超过1000 返回失败
    if(nSize > 1000)
    {
        return FALSE;
    }

    DWORD OldProtect = 0;
    // 这里只改变一页的内存属性
    if(!VirtualProtectEx(hProcess,
                        (void*)address,
                        1,
                        PAGE_NOACCESS,
                        &OldProtect))
    {
        OutputDebugStringA("SetMemBreakPoint->VirtualProtectEx Error!");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        return FALSE;
    }

    MEM_BREAK_POINT membp;
    membp.address = address;
    membp.nSize = nSize;
    membp.OldProtect = OldProtect;
    if(m_pMemBpMap->end() == m_pMemBpMap->find(address) && nSize < 1000)
    {
        m_pMemBpMap->insert(address,membp);
    }

    return TRUE;
}

BOOL CBreakPoint::RestoreMemBeakPoint(HANDLE hProcess, DWORD address)
{
    if(m_pMemBpMap->end() == m_pMemBpMap->find(address))
    {
        return FALSE;
    }

    DWORD Protect = 0;
    if(!VirtualProtectEx(hProcess,
                        (void*)address,
                        m_pMemBpMap->find(address).value().nSize,
                        m_pMemBpMap->find(address).value().OldProtect,
                        &Protect))
    {
        OutputDebugStringA("RestoreMemBeakPoint->VirtualProtectEx Error!");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        return FALSE;
    }

    return TRUE;
}

BOOL CBreakPoint::SetHardBreakPoint(DWORD address, DWORD ThreadId, int nPostion,int nType, int nLength)
{
    // 如果寄存器编号不对 返回错误
    if(nPostion > 4)
    {
        return FALSE;
    }

    // 如果长度不对 直接返回错误
    if(!(nLength == 1 || nLength == 2 || nLength == 4))
    {
        return FALSE;
    }

    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, ThreadId);
    if(nullptr == hThread)
    {
        OutputDebugStringA("SetHardBeakPoint->OpenThread Error");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        return FALSE;
    }

    CONTEXT context;
    context.ContextFlags = CONTEXT_ALL;
    if(!GetThreadContext(hThread, &context))
    {
        OutputDebugStringA("SetHardBeakPoint->GetThreadContext Error");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        CloseHandle(hThread);
        return FALSE;
    }

    if(!SetDrRegister(context,address,nPostion,nType,nLength))
    {
        CloseHandle(hThread);
        return FALSE;
    }

    //OutputDebugStringA(QString::number(context.Dr7, 16).toLocal8Bit());
    //OutputDebugStringA(QString::number(context.Dr1, 16).toLocal8Bit());
    if(!SetThreadContext(hThread, &context))
    {
        OutputDebugStringA("SetHardBeakPoint->SetThreadContext Error");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        CloseHandle(hThread);
        return FALSE;
    }

    CloseHandle(hThread);
    return TRUE;
}

BOOL CBreakPoint::SetDrRegister(CONTEXT &context, DWORD address,int nPostion, int nType,int nLength)
{
    switch (nPostion)
    {
    case 0:
        context.Dr0 = address;
        context.Dr7 |= 0x1;            //L0 = 1
        switch (nType)
        {
        // 执行断点
        case BP_EXECUTION:
            context.Dr7 &= ~0xF0000;
            m_HardBpAry[0].nSize = 0;
            m_HardBpAry[0].nType = BP_EXECUTION;
            m_HardBpAry[0].address = address;
            break;

        // 写入断点
        case BP_WRITE:
            switch (nLength)
            {
            case 1:
                context.Dr7 |= 0x10000;  // 长度1
                break;
            case 2:
                context.Dr7 |= 0x50000;  // 长度2
                break;
            case 3:
                context.Dr7 |= 0xD0000;  // 长度4
                break;
            default:
                return FALSE;
            }
            m_HardBpAry[0].nSize = nLength;
            m_HardBpAry[0].nType = BP_WRITE;
            m_HardBpAry[0].address = address;
            break;

        // 访问断点
        case BP_WRITEREAD:
            switch (nLength)
            {
            case 1:
                context.Dr7 |= 0x30000;  // 长度1
                break;
            case 2:
                context.Dr7 |= 0x70000;  // 长度2
                break;
            case 3:
                context.Dr7 |= 0xF0000;  // 长度4
                break;
            default:
                return FALSE;
            }
            break;
            m_HardBpAry[0].nSize = nLength;
            m_HardBpAry[0].nType = BP_WRITEREAD;
            m_HardBpAry[0].address = address;
        default:
            return FALSE;
        }

        break;
    case 1:
        context.Dr1 = address;
        context.Dr7 |= 0x4;            //L1 = 1
        switch (nType)
        {
        // 执行断点
        case BP_EXECUTION:
            context.Dr7 &= ~0xF00000;   //执行断点
            m_HardBpAry[1].nSize = 0;
            m_HardBpAry[1].nType = BP_EXECUTION;
            m_HardBpAry[1].address = address;
            break;

        // 写入断点
        case BP_WRITE:
            switch (nLength)
            {
            case 1:
                context.Dr7 |= 0x100000;  // 长度1
                break;
            case 2:
                context.Dr7 |= 0x500000;  // 长度2
                break;
            case 3:
                context.Dr7 |= 0xD00000;  // 长度4
                break;
            default:
                return FALSE;
            }
            m_HardBpAry[1].nSize = nLength;
            m_HardBpAry[1].nType = BP_WRITE;
            m_HardBpAry[1].address = address;
            break;

        // 访问断点
        case BP_WRITEREAD:
            switch (nLength)
            {
            case 1:
                context.Dr7 |= 0x300000;  // 长度1
                break;
            case 2:
                context.Dr7 |= 0x700000;  // 长度2
                break;
            case 3:
                context.Dr7 |= 0xF00000;  // 长度4
                break;
            default:
                return FALSE;
            }
            break;
            m_HardBpAry[1].nSize = nLength;
            m_HardBpAry[1].nType = BP_WRITEREAD;
            m_HardBpAry[1].address = address;
        default:
            return FALSE;
        }

        break;
    case 2:
        context.Dr2 = address;
        context.Dr7 |= 0x10;     //L2 = 1
        switch (nType)
        {
        // 执行断点
        case BP_EXECUTION:
            context.Dr7 &= ~0xF000000;   //执行断点
            m_HardBpAry[2].nSize = 0;
            m_HardBpAry[2].nType = BP_EXECUTION;
            m_HardBpAry[2].address = address;
            break;

        // 写入断点
        case BP_WRITE:
            switch (nLength)
            {
            case 1:
                context.Dr7 |= 0x1000000;  // 长度1
                break;
            case 2:
                context.Dr7 |= 0x5000000;  // 长度2
                break;
            case 3:
                context.Dr7 |= 0xD000000;  // 长度4
                break;
            default:
                return FALSE;
            }
            m_HardBpAry[2].nSize = nLength;
            m_HardBpAry[2].nType = BP_WRITE;
            m_HardBpAry[2].address = address;
            break;

        // 访问断点
        case BP_WRITEREAD:
            switch (nLength)
            {
            case 1:
                context.Dr7 |= 0x3000000;  // 长度1
                break;
            case 2:
                context.Dr7 |= 0x7000000;  // 长度2
                break;
            case 3:
                context.Dr7 |= 0xF000000;  // 长度4
                break;
            default:
                return FALSE;
            }
            m_HardBpAry[2].nSize = nLength;
            m_HardBpAry[2].nType = BP_WRITEREAD;
            m_HardBpAry[2].address = address;
            break;
        default:
            return FALSE;
        }
        break;
    case 3:
        context.Dr3 = address;
        context.Dr7 |= 0x40;   //L3 = 1
        switch (nType)
        {
        // 执行断点
        case BP_EXECUTION:
            context.Dr7 &= ~0xF0000000;   //执行断点
            m_HardBpAry[3].nSize = 0;
            m_HardBpAry[3].nType = BP_EXECUTION;
            m_HardBpAry[3].address = address;
            break;

        // 写入断点
        case BP_WRITE:
            switch (nLength)
            {
            case 1:
                context.Dr7 |= 0x10000000;  // 长度1
                break;
            case 2:
                context.Dr7 |= 0x50000000;  // 长度2
                break;
            case 3:
                context.Dr7 |= 0xD0000000;  // 长度4
                break;
            default:
                return FALSE;
            }
            m_HardBpAry[3].nSize = nLength;
            m_HardBpAry[3].nType = BP_WRITE;
            m_HardBpAry[3].address = address;
            break;

        // 访问断点
        case BP_WRITEREAD:
            switch (nLength)
            {
            case 1:
                context.Dr7 |= 0x30000000;  // 长度1
                break;
            case 2:
                context.Dr7 |= 0x70000000;  // 长度2
                break;
            case 3:
                context.Dr7 |= 0xF0000000;  // 长度4
                break;
            default:
                return FALSE;
            }
            m_HardBpAry[3].nSize = nLength;
            m_HardBpAry[3].nType = BP_WRITEREAD;
            m_HardBpAry[3].address = address;
            break;
        default:
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }

    //context.Dr7 |= 0x1000000;            //GE = 1
    return TRUE;

}

BOOL CBreakPoint::RestoreHardBreakPoint(DWORD address, DWORD ThreadId, int nPostion)
{
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, ThreadId);
    if(nullptr == hThread)
    {
        OutputDebugStringA("SetHardBeakPoint->OpenThread Error");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        return FALSE;
    }

    CONTEXT context;
    context.ContextFlags = CONTEXT_ALL;
    if(!GetThreadContext(hThread, &context))
    {
        OutputDebugStringA("RestoreHardBreakPoint->GetThreadContext Error");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        CloseHandle(hThread);
        return FALSE;
    }

    // 对应的寄存器
    switch (nPostion)
    {
    case 0:
        context.Dr7 &= ~0x1;
        break;
    case 1:
        context.Dr7 &= ~0x4;
        break;
    case 2:
        context.Dr7 &= ~0x10;
        break;
    case 3:
        context.Dr7 &= ~0x40;
        break;
    }

    if(!SetThreadContext(hThread, &context))
    {
        OutputDebugStringA("RestoreHardBreakPoint->SetThreadContext Error");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        CloseHandle(hThread);
        return FALSE;
    }

    CloseHandle(hThread);

    return TRUE;
}
