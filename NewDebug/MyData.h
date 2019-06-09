#ifndef MYDATA_H
#define MYDATA_H

#include <Windows.h>
#include <QString>

// 寄存器在UI中tableWidget的编号
#define EAXROW 0
#define EBXROW 1
#define ECXROW 2
#define EDXROW 3
#define EBPROW 4
#define ESPROW 5
#define ESIROW 6
#define EDIROW 7
#define EIPROW 8
#define EFLAGSROW 9
#define ZF 11
#define PF 12
#define AF 13
#define OF 14
#define SF 15
#define DF 16
#define CF 17
#define TF 18
#define IF 19

// 读取内存进行反汇编的字节数
#define MEMCOUNT 1024

// cc断点属性,断点map中保存
typedef struct
{
    // 断点位置
    DWORD address;
    // 原内容
    unsigned char content;
    // 是否是单次断点
    bool frequency;
}BREAK_POINT;

// 内存断点属性,内存断点map中保存
typedef struct
{
    // 断点位置
    DWORD address;
    // 断点长度
    SIZE_T nSize;
    // 原属性
    DWORD OldProtect;
}MEM_BREAK_POINT;

// 硬件断点属性,硬件断点数组中保存
typedef struct
{
    //断点位置
    DWORD address;
    //断点长度
    SIZE_T nSize;
    //断点类型
    int nType;
    //寄存器位置
    int nPostion;
}HARD_BREAK_POINT;

// 导出函数属性
typedef struct
{
    // 导出函数名字
    QString FucName;
    // Dll名字
    QString DllName;
}EXPORT_ADDRESS;

// 断点类型枚举
enum
{
    BP_EXECUTION,  //执行断点
    BP_WRITE,      //写入
    BP_WRITEREAD   //读/写断点
};

// 选项菜单枚举
enum
{
    UI_CPU,
    UI_LOG,
    UI_BPTABEL,
    UI_DLL
};


#endif // MYDATA_H
