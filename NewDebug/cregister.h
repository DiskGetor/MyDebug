#ifndef CREGISTER_H
#define CREGISTER_H

#include <Windows.h>
#include <QObject>

class CRegister : public QObject
{
    Q_OBJECT
public:
    CRegister();

    // 参数1: 传入参数 DEBUG_EVENT  参数2:传出参数 寄存器信息
    BOOL GetRegisters(IN DEBUG_EVENT &Event, OUT CONTEXT &context);

signals:
    //
    void sigShowRegisters(CONTEXT);
};

#endif // CREGISTER_H
