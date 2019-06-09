#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRegExp>
#include "cdebugthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 各种绑定
    void InitBinding();

    // 将寄存器设到UI中
    void setResWidgetTable(int nRow, int nColumn, QString qstrText);

    // 设定标志寄存器
    void setResFlag(unsigned long ulflags);

    // 在UI中标记断点
    void SignBreakPoint();

    // 转换相对地址
    // 参数1:指令IP地址
    // 参数1:第一个指令码
    // 参数2:反汇编字符串的指针
    void TransformRelAddress(unsigned int nAddress,QString Hex,QString &Disassembly);

    // 断点表
    void BreakPointTable();

    // 加载的DLL表
    void LoadDllTable();

    // 通过地址查询函数名字
    bool FindFucName(DWORD Address,QString &FucName);
private slots:
    // 输入完成信号
    void on_lineEdit_Cmd_editingFinished();

    // 选择表的菜单的信号
    void on_tabWidget_currentChanged(int index);

    // 接收寄存器的信息
    void sltShowRegisters(CONTEXT);

    // 接收反汇编信息
    void sltDisAssemblyData(unsigned int nAddress, QString qstrHex,QString qstrData);

    // 接收内存信息
    void sltMemInfo(QByteArray qbtaMem, unsigned int nLength, unsigned int nAddress);


signals:
    // 发送命令行消息
    void sigCmdMessage(QString);

private:
    Ui::MainWindow *ui;

    CDebugThread *m_pDebugThread = new CDebugThread;
};

#endif // MAINWINDOW_H
