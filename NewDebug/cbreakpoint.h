#ifndef CBREAKPOINT_H
#define CBREAKPOINT_H

#include <QObject>
#include <Windows.h>
#include <QMap>
#include "MyData.h"


class CBreakPoint : public  QObject
{
    Q_OBJECT
public:
    CBreakPoint();

    //--------------------------------cc断点----------------------------------//
    // 设置cc断点函数
    // 参数1:进程句柄
    // 参数2:断点地址
    // 参数3:是否是单次断点
    // 返回值: FALSE失败  TRUE成功
    BOOL SetBreakPoint(IN HANDLE hProcess, IN DWORD address, IN bool frequency);

    // 还原断点函数
    // 参数1:进程句柄
    // 参数2:断点地址
    // 返回值: FALSE失败  TRUE成功
    BOOL RestoreBeakPoint(IN HANDLE hProcess, IN DWORD address);
    //------------------------------------------------------------------------//

    //--------------------------------内存断点---------------------------------//
    // 设置内存断点函数
    // 参数1:进程句柄
    // 参数2:断点地址
    // 参数3:设置长度
    // 返回值: FALSE失败  TRUE成功
    BOOL SetMemBreakPoint(IN HANDLE hProcess, IN DWORD address, IN SIZE_T nSize);

    // 还原内存断点
    // 参数1:进程句柄
    // 参数2:断点地址
    // 返回值: FALSE失败  TRUE成功
    BOOL RestoreMemBeakPoint(IN HANDLE hProcess, IN DWORD address);
    //------------------------------------------------------------------------//

    //--------------------------------硬件断点---------------------------------//
    // 设置硬件断点
    // 参数1:断点地址
    // 参数2:线程ID
    // 参数3:Dr寄存器编号 0-3 其余数字返回失败
    // 参数4:断点类型
    // 参数5:断点长度 如果断点类型是执行断点 则忽略此参数 如果不是执行断点只接受1,2,4其余返回失败
    // 返回值: FALSE失败  TRUE成功
    BOOL SetHardBreakPoint(IN DWORD address, IN DWORD ThreadId,IN int nPostion,IN int nType, IN int nLength);

    // 修改Dr寄存器
    // 参数1:CONTEXT的引用
    // 参数2:断点地址
    // 参数3:Dr寄存器编号 0-3 其余数字返回失败
    // 参数4:断点类型
    // 参数5:断点长度 如果断点类型是执行断点 则忽略此参数 如果不是执行断点只接受1,2,4其余返回失败
    BOOL SetDrRegister(IN CONTEXT &context, IN DWORD address,IN int nPostion, int nType, int nLength);

    // 恢复硬件断点
    // 参数1:断点地址
    // 参数2:线程Id
    // 参数3:Dr寄存器编号 0-3 其余数字返回失败
    BOOL RestoreHardBreakPoint(IN DWORD address , DWORD ThreadId, int nPostion);
public:
    // cc断点表
    QMap<DWORD,BREAK_POINT> *m_pBpMap = new QMap<DWORD,BREAK_POINT>;

    // 内存断点表
    QMap<DWORD,MEM_BREAK_POINT> *m_pMemBpMap = new QMap<DWORD,MEM_BREAK_POINT>;

    // 硬件断点表
    HARD_BREAK_POINT m_HardBpAry[4];

};

#endif // CBREAKPOINT_H
