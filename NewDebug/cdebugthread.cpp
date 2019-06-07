#include "cdebugthread.h"

void CDebugThread::run()
{
    // 如果没有加载成功 打印日志
    if(!InitDebug())
    {
        PrintLog("InitDebug error!");
    }
}

int CDebugThread::InitDebug()
{

    STARTUPINFOA si = STARTUPINFOA();
    PROCESS_INFORMATION pi;
    si.cb = sizeof(si);

    if (!CreateProcessA(nullptr,
                      (LPSTR)"TestAsm.exe",
                      nullptr,
                      nullptr,
                      FALSE,
                      DEBUG_ONLY_THIS_PROCESS,
                      nullptr,
                      nullptr,
                      &si,
                      &pi))
    {
         PrintLog("debug error");
         PrintLog(QString::number(GetLastError()));
         return FALSE;
    }
    PrintLog("debug ok\n");

    m_hProcess = pi.hProcess;

    do
    {
        //等待调试器事件
        if (!WaitForDebugEvent(&m_Event, INFINITE))
        {
          PrintLog("WaitForDebugEvent error\n");
          PrintLog(QString::number(GetLastError()));
          continue;
        }

        DWORD dwContinueStatus;

        dwContinueStatus = CDebugThread::HandlerEvnent(m_Event);
        if(0 == ContinueDebugEvent(m_Event.dwProcessId, m_Event.dwThreadId, dwContinueStatus))
        {
          PrintLog("ContinueDebugEvent Error");
          PrintLog(QString::number(GetLastError()));
          break;
        }
    }
    while (TRUE);

    return TRUE;
}

void CDebugThread::PrintLog(QString qstrStr)
{
    qDebug() << qstrStr;
}

BOOL CDebugThread::SetStep()
{
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, m_Event.dwThreadId);

    CONTEXT context;
    context.ContextFlags = CONTEXT_FULL;
    if(!GetThreadContext(hThread, &context))
    {
        PrintLog("GetThreadContext error");
        PrintLog(QString::number(GetLastError()));
        return FALSE;
    }

    context.EFlags |= 0x100;

    if(!SetThreadContext(hThread, &context))
    {
        PrintLog("SetThreadContext error");
        PrintLog(QString::number(GetLastError()));
        return FALSE;
    }

    CloseHandle(hThread);
    return TRUE;
}

BOOL CDebugThread::ReadMemory(DWORD ulAdderss, unsigned int nLength)
{
    if(!m_pCMemory->GetTargetMemory(m_hProcess,m_Event.dwThreadId,ulAdderss,nLength))
    {
        PrintLog("ReadMemory Error!");
        return FALSE;
    }

    return TRUE;
}

DWORD CDebugThread::HandlerEvnent(DEBUG_EVENT &Event)
{

    switch (Event.dwDebugEventCode)
    {
    case EXCEPTION_DEBUG_EVENT:
        return OnException();

    case CREATE_THREAD_DEBUG_EVENT:
        return OnCreateThread();

    case CREATE_PROCESS_DEBUG_EVENT:
        return OnCreateProcess();

    case EXIT_THREAD_DEBUG_EVENT:
        return OnExitProcess();

    case EXIT_PROCESS_DEBUG_EVENT:
        return OnExitThread();

    case LOAD_DLL_DEBUG_EVENT:
        return OnLoadDll();

    case UNLOAD_DLL_DEBUG_EVENT:
        return OnUnLoadDll();

    case OUTPUT_DEBUG_STRING_EVENT:
        return OnOutputDebugString();

    case RIP_EVENT:
        break;
    }


    //return DBG_EXCEPTION_HANDLED;
    return DBG_CONTINUE;
}

DWORD CDebugThread::OnException()
{
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

    PrintLog("OnException\n");
    EXCEPTION_RECORD record = m_Event.u.Exception.ExceptionRecord;

    // 更新到当前Eip
    CONTEXT context = CONTEXT();
    m_pCRegistr->GetRegisters(m_Event,context);
    m_CurEIP = context.Eip;

    switch (record.ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION: //内存访问异常
      dwContinueStatus = OnExceAssess();
      break;
    case EXCEPTION_BREAKPOINT: //断点异常
      dwContinueStatus = OnExceBreakPoint(context);
      break;
    case EXCEPTION_SINGLE_STEP: //单步异常
      dwContinueStatus = OnExceSingleStep();
      break;
    case EXCEPTION_ILLEGAL_INSTRUCTION://无效指令
      break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO: //除零
      break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
      break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
      break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      break;
    case EXCEPTION_FLT_INEXACT_RESULT:
      break;
    case EXCEPTION_FLT_INVALID_OPERATION:
      break;
    case EXCEPTION_FLT_OVERFLOW:
      break;
    case EXCEPTION_FLT_STACK_CHECK:
      break;
    case EXCEPTION_FLT_UNDERFLOW:
      break;
    case EXCEPTION_IN_PAGE_ERROR:
      break;
    case EXCEPTION_INT_OVERFLOW:
      break;
    case EXCEPTION_INVALID_DISPOSITION:
      break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      break;
    case EXCEPTION_PRIV_INSTRUCTION:
      break;
    case EXCEPTION_STACK_OVERFLOW:
      break;
    }


    // 获取寄存器信息
    m_pCRegistr->sigShowRegisters(context);
    // 获取反汇编信息
    if(FALSE == m_pCDisassembly->StartDisassembly(m_hProcess,m_CurEIP,80,*m_pCBreakPoint->m_pBpMap))
    {
        PrintLog("Disassembly Error!");
    }
    while(1)
    {
        __asm
        {
            push eax
            pop  eax
        }
        if(m_bBlocking == FALSE)
        {
            break;
        }
    }
    m_bBlocking = TRUE;
    return dwContinueStatus;
}

DWORD CDebugThread::OnCreateProcess()
{
    PrintLog("OnCreateProcess baseImage=%p\n");
    return DBG_CONTINUE;
}

DWORD CDebugThread::OnCreateThread()
{
    PrintLog("OnCreateThread id:%d handle:%p\n");
    return DBG_CONTINUE;
}

DWORD CDebugThread::OnExitThread()
{
    PrintLog("OnExitThread\n");
    return DBG_CONTINUE;
}

DWORD CDebugThread::OnExitProcess()
{
    PrintLog("OnExitProcess\n");
    return DBG_CONTINUE;
}

DWORD CDebugThread::OnLoadDll()
{
    PrintLog("OnLoadDll\n");
    return DBG_CONTINUE;
}

DWORD CDebugThread::OnUnLoadDll()
{
    PrintLog("OnUnLoadDll\n");
    return DBG_CONTINUE;
}

DWORD CDebugThread::OnOutputDebugString()
{
    PrintLog("OnOutputDebugString\n");
    return DBG_CONTINUE;
}

DWORD CDebugThread::OnExceAssess()
{
    PrintLog("OnExceAssess");

    EXCEPTION_RECORD record = m_Event.u.Exception.ExceptionRecord;
    m_bBlocking = FALSE;
    //遍历内存断点表 看断点是否命中用户指定的区域
    for (auto it = m_pCBreakPoint->m_pMemBpMap->begin(); it != m_pCBreakPoint->m_pMemBpMap->end(); ++it)
    {
        // 断点是否命中用户断点区
        if((it->address & ~0xfff) <= record.ExceptionInformation[1] &&
           ((it->address & ~0xfff) + 0x1000) >= record.ExceptionInformation[1])
        {
            m_FixMemAddress.address = it->address;
            m_FixMemAddress.OldProtect = it->OldProtect;
        }

        // 断点是否命中用户断点区
        if(it->address <= record.ExceptionInformation[1] &&
           it->address + it->nSize >= record.ExceptionInformation[1])
        {
            // 命中 不放行
            m_bBlocking = TRUE;
            m_FixMemAddress.address = it->address;
            m_FixMemAddress.OldProtect = it->OldProtect;
            break;
        }

    }


    qDebug("%p",record.ExceptionInformation[1]);
    //修复内存页属性
    m_pCBreakPoint->RestoreMemBeakPoint(m_hProcess,m_FixMemAddress.address);

    //设置单步 用于过内存断点后还原内存断点
    SetStep();

    return DBG_CONTINUE;
}

DWORD CDebugThread::OnExceBreakPoint(CONTEXT &context)
{
    PrintLog("OnExceBreakPoint");

    // 第一断点修改此属性 后面不再进入
    // 系统断点更改 m_FirstBreakPoint = TRUE 则可以直接忽略系统断点
    if(m_FirstBreakPoint)
    {
        m_FirstBreakPoint = FALSE;
        m_bBlocking = FALSE;
        PrintLog("Reach System breakpoint");
        return DBG_CONTINUE;
    }


    // 如果在断点表中找到了用户下的断点
    if(m_pCBreakPoint->m_pBpMap->end() != m_pCBreakPoint->m_pBpMap->find(m_CurEIP - 1))
    {
        // 修复ip地址
        HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, m_Event.dwThreadId);
        if(nullptr == hThread)
        {
            PrintLog("OpenThread Error!");
            PrintLog(QString::number(GetLastError()));
            return DBG_EXCEPTION_NOT_HANDLED;
        }

        context.Eip -= 1;
        --m_CurEIP;
        if(!SetThreadContext(hThread, &context))
        {
            PrintLog("SetThreadContext Error!");
            PrintLog(QString::number(GetLastError()));
            return DBG_EXCEPTION_NOT_HANDLED;
        }
        CloseHandle(hThread);

        // 修复断点
        m_pCBreakPoint->RestoreBeakPoint(m_hProcess,m_CurEIP);

        // 判断是否是单次断点 单次断点从断点表中摘除
        if(m_pCBreakPoint->m_pBpMap->find(m_CurEIP).value().frequency)
        {
            m_pCBreakPoint->m_pBpMap->erase(m_pCBreakPoint->m_pBpMap->find(m_CurEIP));
            return DBG_CONTINUE;
        }
        // 单步标志 步过后断点地址继续填cc
        m_FixBpAddress = m_CurEIP;
        return DBG_CONTINUE;
    }

    //return DBG_CONTINUE;
    return DBG_CONTINUE;
}

DWORD CDebugThread::OnExceSingleStep()
{
    DWORD DBG_STUTAS = DBG_CONTINUE;
    PrintLog("OnExceSingleStep");

    // 如果是修复断点引起的异常
    if(0 != m_FixBpAddress)
    {
        m_pCBreakPoint->SetBreakPoint(m_hProcess,m_FixBpAddress,true);
        m_FixBpAddress = 0;
        m_bBlocking = FALSE;
        return DBG_CONTINUE;
    }

    // 如果是修复内存访问引起的异常
    if(0 != m_FixMemAddress.address)
    {
        // 1000代表是修复异常用 不会加入到断点表中
        m_pCBreakPoint->SetMemBreakPoint(m_hProcess,m_FixMemAddress.address,1000);
        m_FixMemAddress.address = 0;
        m_bBlocking = FALSE;
        return DBG_CONTINUE;
    }

    // 如果是修复硬件断点的异常
    if(0 != m_FixHardAddress.address)
    {

        m_pCBreakPoint->SetHardBreakPoint(m_FixHardAddress.address,
                                          m_Event.dwThreadId,
                                          m_FixHardAddress.nPostion,
                                          m_FixHardAddress.nType,
                                          m_FixHardAddress.nSize);
        m_FixHardAddress.address = 0;
        m_bBlocking = FALSE;

        return DBG_CONTINUE;
    }

    // 如果是硬件断点异常
    for(int i = 0; i < 4 ; ++ i)
    {

        if (m_CurEIP == m_pCBreakPoint->m_HardBpAry[i].address)
        {
            qDebug() << "Fix Hard";
            // 取消硬件断点 设置单步 跑完这句重新设置回去
            m_FixHardAddress = m_pCBreakPoint->m_HardBpAry[i];
            m_FixHardAddress.nPostion = i;
            m_pCBreakPoint->RestoreHardBreakPoint(m_CurEIP,m_Event.dwThreadId,i);
            return DBG_CONTINUE;
        }

    }


    // 进单步异常判断是否是自己设置的单步
    // 是就返回DBG_CONTINUE  不是 不处理异常
    if(m_bSingleEip)
    {
        DBG_STUTAS = DBG_CONTINUE;
    }
    else
    {
        DBG_STUTAS = DBG_EXCEPTION_NOT_HANDLED;
    }

    m_bSingleEip = FALSE;
    return DBG_STUTAS;
}

void CDebugThread::sltCmdMessage(QString qstrCmd)
{
    // 单步命令
    if(0 == qstrCmd.compare(QString::fromLocal8Bit("t")))
    {
        if(SetStep())
        {
            m_bSingleEip = TRUE;
            m_bBlocking = FALSE;
            return;
        }
        else
        {
            PrintLog("SetStep Error!");
        }
    }
    // go命令
//    else if(0 == qstrCmd.compare(QString::fromLocal8Bit("g")))
//    {
//        m_bBlocking = FALSE;
//        return;
//    }

    // go命令
    else if(0 == qstrCmd.mid(0,1).compare(QString::fromLocal8Bit("g")))
    {
        auto qstrlist = qstrCmd.split(' ');

        // 长度为2  并且第2个参数有写
        if(2 == qstrlist.size() && 0 != qstrlist.at(1).size())
        {

            if(!m_pCBreakPoint->SetBreakPoint(m_hProcess,qstrlist.at(1).toUInt(nullptr, 16),true))
            {
                PrintLog("Set BreakPoint Error!");
                return;
            }
            else
            {
                qDebug("%p",qstrlist.at(1).toUInt(nullptr, 16));
                PrintLog("Set BreakPoint Success!");
                m_bBlocking = FALSE;
                return;
            }
        }
        else if(1 == qstrlist.size())
        {
           m_bBlocking = FALSE;
           return;
        }

        PrintLog("Invalid instruction!");
        return;
    }

    // cc断点
    // bp 0x123456       (1)
    // bp  地址    填参数3表示单次断点
    else if(0 == qstrCmd.mid(0,2).compare(QString::fromLocal8Bit("bp")))
    {
        auto qstrlist = qstrCmd.split(' ');
        if(2 == qstrlist.size() && 0 != qstrlist.at(1).size())
        {
            if(!m_pCBreakPoint->SetBreakPoint(m_hProcess,qstrlist.at(1).toUInt(nullptr, 16),false))
            {
                PrintLog("Set BreakPoint Error!");
            }
            else
            {
                PrintLog("Set BreakPoint Success!");
            }
        }
        else if(3 == qstrlist.size() && 0 != qstrlist.at(1).size())
        {
            if(!m_pCBreakPoint->SetBreakPoint(m_hProcess,qstrlist.at(1).toUInt(nullptr, 16),true))
            {
                PrintLog("Set SingleBreakPoint Error!");
            }
            else
            {
                PrintLog("Set SingleBreakPoint Success!");
            }
        }
        return;
    }
    // 硬件断点
    // ba 0x123456      0           0                 1
    // ba  地址      Dr0寄存器  类型(MyData里)   长度(只支持1,2,4)
    else if(0 == qstrCmd.mid(0,2).compare(QString::fromLocal8Bit("ba")))
    {
        auto qstrlist = qstrCmd.split(' ');
        if(5 == qstrlist.size() && 0 != qstrlist.at(1).size())
        {
            if(!m_pCBreakPoint->SetHardBreakPoint(qstrlist.at(1).toUInt(nullptr, 16),
                                                  m_Event.dwThreadId,
                                                  qstrlist.at(2).toInt(),
                                                  qstrlist.at(3).toInt(),
                                                  qstrlist.at(4).toInt()))
            {
                PrintLog("SetHardBreakPoint Error!");
            }
            else
            {
                PrintLog("SetHardBreakPoint Success!");
            }
        }
        return;
    }

    // 内存断点
    else if(0 == qstrCmd.mid(0,2).compare(QString::fromLocal8Bit("mr")))
    {
        auto qstrlist = qstrCmd.split(' ');
        if(3 == qstrlist.size() && 0 != qstrlist.at(1).size())
        {
            if(!m_pCBreakPoint->SetMemBreakPoint(m_hProcess,
                                                 qstrlist.at(1).toUInt(nullptr, 16),
                                                 qstrlist.at(2).toUInt()))
            {
                PrintLog("SetMemBreakPoint Error!");
            }
            else
            {
                PrintLog("SetMemBreakPoint Success!");
            }
        }
    }

    // 反汇编
    else if(0 == qstrCmd.compare(QString::fromLocal8Bit("u")))
    {
        m_pCDisassembly->StartDisassembly(m_hProcess,m_CurEIP,80,*m_pCBreakPoint->m_pBpMap);
        return;
    }

    else if(0 == qstrCmd.compare(QString::fromLocal8Bit("d")))
    {
       ReadMemory(m_CurEIP,80);
    }
}
