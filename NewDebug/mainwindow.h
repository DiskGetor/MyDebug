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


private slots:
    void on_lineEdit_Cmd_editingFinished();

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
