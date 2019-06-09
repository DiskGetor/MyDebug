#ifndef CDEBUGTHREAD_H
#define CDEBUGTHREAD_H

#include <QThread>
#include <Windows.h>
#include <Psapi.h>
#include <QDebug>
#include <QByteArray>
#include <QMap>
#include "MyData.h"
#include "cregister.h"
#include "cdisassembly.h"
#include "cbreakpoint.h"
#include "cmemory.h"

#pragma  comment(lib, "Psapi.lib")
#pragma  comment(lib, "User32.lib")
#pragma  comment(lib, "kernel32.lib")

class CDebugThread : public QThread
{
    Q_OBJECT
public:

    virtual void run();

    // 初始化Debug
    int InitDebug();

    // Debug事件回调函数
    DWORD HandlerEvnent(DEBUG_EVENT& Event);

    // 打印日志
    void PrintLog(QString qstrStr);


    // 设置单步
    BOOL SetStep();
    //---------------内存函数-------------------//
    // 读取内存
    // 参数1:读取地址   参数2:读取长度
    // 返回值 成功读取(TRUE) 或读取失败(FALSE)
    BOOL ReadMemory(DWORD ulAdderss, unsigned int nLength);

    //---------------事件函数-------------------//
    // 异常事件
    DWORD OnException();

    // 创建进程事件
    DWORD OnCreateProcess();

    // 创建线程事件
    DWORD OnCreateThread();

    // 退出线程事件
    DWORD OnExitThread();

    // 退出进程事件
    DWORD OnExitProcess();

    // 加载Dll事件
    DWORD OnLoadDll();

    // 卸载Dll事件
    DWORD OnUnLoadDll();

    // 打印日志事件
    DWORD OnOutputDebugString();
    //--------------------------------------//

    //---------------异常处理-------------------//
    // 内存访问异常
    DWORD OnExceAssess();

    // 断点异常
    DWORD OnExceBreakPoint(CONTEXT &context);

    // 单步异常
    DWORD OnExceSingleStep();
    //--------------------------------------//

public slots:
    // 接收命令行信息
    void sltCmdMessage(QString qstrCmd);

private:
    // 被调试程序的HANDLE
    HANDLE m_hProcess = nullptr;

    // DEBUG_EVENT
    DEBUG_EVENT m_Event;

    // 阻塞标志
    BOOL m_bBlocking = TRUE;

    // 断点表
    QMap<DWORD,BREAK_POINT> *pBpMap = new QMap<DWORD,BREAK_POINT>;

    // 当前EIP
    DWORD m_CurEIP = 0;

    // 是否开启系统系统断点
    BOOL m_FirstBreakPoint = FALSE;

    // 是否下过单步断点
    BOOL m_bSingleEip = FALSE;

    // 如果不是m_FixBpAddress != 0 则是修复断点引起的单步
    DWORD m_FixBpAddress = 0;

    // m_FixMemAddress != 0 则是修复内存访问引起的单步
    MEM_BREAK_POINT m_FixMemAddress = MEM_BREAK_POINT();

    // 如果m_FixHardAddress != 0 则是修复硬件断点引起的单步
    HARD_BREAK_POINT m_FixHardAddress = HARD_BREAK_POINT();
    //--------------------------------------//
public:
    // 寄存器类
    CRegister *m_pCRegistr = new CRegister;
    // 反汇编类
    CDisassembly *m_pCDisassembly = new CDisassembly;
    // cc断点类
    CBreakPoint *m_pCBreakPoint = new CBreakPoint;
    // 内存读取类
    CMemory *m_pCMemory = new CMemory;
    // DLL导出函数 地址为键  存函数名称
    QMap<DWORD, EXPORT_ADDRESS> *m_pFucNameMap = new QMap<DWORD, EXPORT_ADDRESS>;
    // DLL导出函数 Dll载入首地址为键 存DLL名字
    QMap<DWORD,QString> *m_pDllMap = new QMap<DWORD,QString>;
};

#endif // CDEBUGTHREAD_H
