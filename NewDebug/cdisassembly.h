#ifndef CDISASSEMBLY_H
#define CDISASSEMBLY_H

#include <Windows.h>
#include <QObject>
#include <QByteArray>
#include <inttypes.h>
#include <Zydis/Zydis.h>
#include <Zycore/Types.h>
#include <Zycore/Status.h>
#include <QMap>
#include "MyData.h"

#pragma  comment(lib, "Zydis.lib")

class CDisassembly : public QObject
{
    Q_OBJECT
public:
    CDisassembly();

    // 参数1: 进程句柄   参数2:当前Eip  参数3:解析的字节数  参数4:断点表的引用
    int StartDisassembly(IN HANDLE hProcess,IN DWORD Eip, IN DWORD nLength, IN QMap<DWORD,BREAK_POINT> &pBpMap);

signals:
    // 发送反汇编数据到UI线程
    // 参数1:地址    参数2:机器码    参数3:反汇编
    void sigDisAssemblyData(unsigned int, QString ,QString);
};

#endif // CDISASSEMBLY_H
