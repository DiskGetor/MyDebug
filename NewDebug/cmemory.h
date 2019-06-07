#ifndef CMEMORY_H
#define CMEMORY_H

#include <QObject>
#include <Windows.h>
#include <QByteArray>


class CMemory : public QObject
{
    Q_OBJECT
public:
    CMemory();

    // 读取进程内存数据
    // 参数1: 读取地址   参数2: 读取长度
    BOOL GetTargetMemory(HANDLE hProcess, DWORD dwThreadId,unsigned int address, unsigned int length);
signals:
    // 发送内存信息到UI

    void sigMemInfo(QByteArray , unsigned int, unsigned int);
};

#endif // CMEMORY_H
