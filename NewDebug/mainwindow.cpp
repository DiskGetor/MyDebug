#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 注册CONTEXT类型用于信号槽传递
    qRegisterMetaType<CONTEXT>("CONTEXT");

    InitBinding();

    m_pDebugThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitBinding()
{
    // 绑定CMD 发送到debug线程
    QObject::connect(this,&MainWindow::sigCmdMessage,
                     m_pDebugThread,&CDebugThread::sltCmdMessage);

    //绑定 接收寄存器信息
    QObject::connect(m_pDebugThread->m_pCRegistr,&CRegister::sigShowRegisters,
                     this,&MainWindow::sltShowRegisters);

    //绑定 接收反汇编信息
    QObject::connect(m_pDebugThread->m_pCDisassembly,&CDisassembly::sigDisAssemblyData,
                     this,&MainWindow::sltDisAssemblyData);

    //绑定 接收内存信息
    QObject::connect(m_pDebugThread->m_pCMemory,&CMemory::sigMemInfo,
                     this,&MainWindow::sltMemInfo);
}

void MainWindow::setResWidgetTable(int nRow, int nColumn, QString qstrText)
{
    ui->tableWidget_Register->setItem(nRow,nColumn,new QTableWidgetItem(qstrText));
}

void MainWindow::setResFlag(unsigned long ulflags)
{
    // 标志寄存器

    if(ulflags & 0x1)
    {
       setResWidgetTable(CF,0,QString::number(1));
    }
    else
    {
        setResWidgetTable(CF,0,QString::number(0));
    }

    if(ulflags & 0x4)
    {
       setResWidgetTable(PF,0,QString::number(1));
    }
    else
    {
        setResWidgetTable(PF,0,QString::number(0));
    }

    if(ulflags & 0x10)
    {
       setResWidgetTable(AF,0,QString::number(1));
    }
    else
    {
        setResWidgetTable(AF,0,QString::number(0));
    }

    if(ulflags & 0x40)
    {
       setResWidgetTable(ZF,0,QString::number(1));
    }
    else
    {
        setResWidgetTable(ZF,0,QString::number(0));
    }

    if(ulflags & 0x80)
    {
       setResWidgetTable(SF,0,QString::number(1));
    }
    else
    {
        setResWidgetTable(SF,0,QString::number(0));
    }

    if(ulflags & 0x100)
    {
       setResWidgetTable(TF,0,QString::number(1));
    }
    else
    {
        setResWidgetTable(TF,0,QString::number(0));
    }

    if(ulflags & 0x200)
    {
       setResWidgetTable(IF,0,QString::number(1));
    }
    else
    {
        setResWidgetTable(IF,0,QString::number(0));
    }

    if(ulflags & 0x400)
    {
       setResWidgetTable(DF,0,QString::number(1));
    }
    else
    {
        setResWidgetTable(DF,0,QString::number(0));
    }

    if(ulflags & 0x800)
    {
       setResWidgetTable(OF,0,QString::number(1));
    }
    else
    {
        setResWidgetTable(OF,0,QString::number(0));
    }
}

void MainWindow::SignBreakPoint()
{
    int nRowCount = ui->tableWidget_Disassembly->rowCount();
    for(int i = 0; i < nRowCount; ++i)
    {
        DWORD nAddress = ui->tableWidget_Disassembly->item(i,0)->text().toUInt(nullptr,16);
        if(m_pDebugThread->m_pCBreakPoint->m_pBpMap->end() != m_pDebugThread->m_pCBreakPoint->m_pBpMap->find(nAddress))
        {
            ui->tableWidget_Disassembly->item(i,0)->setBackground(Qt::red);
        }
    }

}

void MainWindow::TransformRelAddress(unsigned int nAddress,QString Hex, QString &Disassembly)
{

    if(Hex != "e8" && Hex !="eb" && Hex != "74"&& Hex != "75" &&
       Hex != "e9")
    {
        return;
    }

    auto qstrlist = Disassembly.split('x');

    nAddress += qstrlist.at(1).toLongLong(nullptr,16);

    Disassembly = qstrlist.at(0);
    Disassembly += 'x';
    Disassembly += QString::number(nAddress,16);
}

void MainWindow::BreakPointTable()
{
    // 清空表重新遍历
    ui->tableWidget_BpTable->setRowCount(0);

    // 遍历CC断点表
    auto ccBreak = m_pDebugThread->m_pCBreakPoint->m_pBpMap;
    for (auto it = ccBreak->begin(); it != ccBreak->end(); ++it)
    {
        // 插入一行
        ui->tableWidget_BpTable->insertRow(ui->tableWidget_BpTable->rowCount());

        // 填类型
        ui->tableWidget_BpTable->setItem(ui->tableWidget_BpTable->rowCount() - 1, 0, new QTableWidgetItem(QString::fromLocal8Bit("cc断点")));
        // 填地址
        ui->tableWidget_BpTable->setItem(ui->tableWidget_BpTable->rowCount() - 1, 1, new QTableWidgetItem(QString::number(it.value().address,16)));
    }

    // 遍历内存断点表
    auto MemBreak = m_pDebugThread->m_pCBreakPoint->m_pMemBpMap;
    for (auto it = MemBreak->begin(); it != MemBreak->end(); ++it)

    {   // 插入一行
        ui->tableWidget_BpTable->insertRow(ui->tableWidget_BpTable->rowCount());
        // 填类型
        // 填类型
        QString Type = QString::fromLocal8Bit("硬件");

        // 填类型
        ui->tableWidget_BpTable->setItem(ui->tableWidget_BpTable->rowCount() - 1, 0, new QTableWidgetItem(QString::fromLocal8Bit("内存断点")));
        // 填地址
        ui->tableWidget_BpTable->setItem(ui->tableWidget_BpTable->rowCount() - 1, 1, new QTableWidgetItem(QString::number(it.value().address,16)));
    }

    // 遍历硬件断点表
    auto hardBreak = m_pDebugThread->m_pCBreakPoint->m_HardBpAry;
    for(int i = 0; i < 4; ++i)
    {
       if(0 != hardBreak[i].address)
       {
           // 插入一行
           ui->tableWidget_BpTable->insertRow(ui->tableWidget_BpTable->rowCount());

           // 填类型
           QString Type = QString::fromLocal8Bit("硬件");
           switch (hardBreak[i].nType)
           {
           case BP_WRITE:
               Type += QString::fromLocal8Bit("写入断点");
               break;
           case BP_EXECUTION:
               Type += QString::fromLocal8Bit("执行断点");
               break;
           case BP_WRITEREAD:
               Type += QString::fromLocal8Bit("读写断点");
               break;
           }
           ui->tableWidget_BpTable->setItem(ui->tableWidget_BpTable->rowCount() - 1, 0, new QTableWidgetItem(Type));

           // 填地址
           ui->tableWidget_BpTable->setItem(ui->tableWidget_BpTable->rowCount() - 1, 1, new QTableWidgetItem(QString::number(hardBreak[i].address,16)));

           // 填长度
           ui->tableWidget_BpTable->setItem(ui->tableWidget_BpTable->rowCount() - 1, 2, new QTableWidgetItem(QString::number(hardBreak[i].nSize)));
       }
    }

}

void MainWindow::LoadDllTable()
{
    ui->tableWidget_DLL->setRowCount(0);

    // 遍历DLLMap
    auto it = m_pDebugThread->m_pDllMap->begin();
    for( ; it != m_pDebugThread->m_pDllMap->end(); ++it)
    {
        // 加一行
        ui->tableWidget_DLL->insertRow(ui->tableWidget_DLL->rowCount());

        ui->tableWidget_DLL->setItem(ui->tableWidget_DLL->rowCount() - 1,0,new QTableWidgetItem(it.value()));
        ui->tableWidget_DLL->setItem(ui->tableWidget_DLL->rowCount() - 1,1,new QTableWidgetItem(QString::number(it.key(),16)));
    }
}

bool MainWindow::FindFucName(DWORD Address ,QString &FucName)
{
    // 查找函数
    auto it = m_pDebugThread->m_pFucNameMap->find(Address);
    if(it != m_pDebugThread->m_pFucNameMap->end())
    {
        FucName = it.value().FucName;
        return true;
    }

    return false;
}

void MainWindow::on_lineEdit_Cmd_editingFinished()
{
    // 空数据不发送
    if(0 == ui->lineEdit_Cmd->text().size())
    {
        return;
    }

    // 如果步过指令下一行是call 步过指令在call下一行下单次cc断点 然后g
    // 如果不是call 按单步指令处理
    if(0 == ui->lineEdit_Cmd->text().compare("p"))
    {
        if(ui->tableWidget_Disassembly->item(0,2)->text().contains("call"))
        {

            // bp xxxxx 1
            QString Cmd = "bp ";
            Cmd += ui->tableWidget_Disassembly->item(1,0)->text();
            Cmd += " 1";
            ui->lineEdit_Cmd->setText(Cmd);
            // 发送Cmd输入的文本到Debug线程
            ui->tableWidget_Disassembly->setRowCount(0);
            emit sigCmdMessage(ui->lineEdit_Cmd->text());
            emit sigCmdMessage("g");
            // 清空文本框
            ui->lineEdit_Cmd->clear();
            return;
        }
        else
        {
            ui->lineEdit_Cmd->setText("t");
        }
    }

    // 单步时或者g时 清空反汇编
    if(0 == ui->lineEdit_Cmd->text().compare("t") ||
       0 == ui->lineEdit_Cmd->text().compare("p") ||
       0 == ui->lineEdit_Cmd->text().mid(0,1).compare("g") ||
       0 == ui->lineEdit_Cmd->text().compare("u"))
    {
        ui->tableWidget_Disassembly->setRowCount(0);
    }

    // 发送Cmd输入的文本到Debug线程
    emit sigCmdMessage(ui->lineEdit_Cmd->text());
    // 清空文本框
    ui->lineEdit_Cmd->clear();
}

void MainWindow::sltShowRegisters(CONTEXT context)
{
    setResWidgetTable(EAXROW,0,QString("0x%1").arg(uint(context.Eax), 8, 16, QLatin1Char('0')));
    setResWidgetTable(EBXROW,0,QString("0x%1").arg(uint(context.Ebx), 8, 16, QLatin1Char('0')));
    setResWidgetTable(ECXROW,0,QString("0x%1").arg(uint(context.Ecx), 8, 16, QLatin1Char('0')));
    setResWidgetTable(EDXROW,0,QString("0x%1").arg(uint(context.Edx), 8, 16, QLatin1Char('0')));
    setResWidgetTable(EBPROW,0,QString("0x%1").arg(uint(context.Ebp), 8, 16, QLatin1Char('0')));
    setResWidgetTable(ESPROW,0,QString("0x%1").arg(uint(context.Esp), 8, 16, QLatin1Char('0')));
    setResWidgetTable(ESIROW,0,QString("0x%1").arg(uint(context.Esi), 8, 16, QLatin1Char('0')));
    setResWidgetTable(EDIROW,0,QString("0x%1").arg(uint(context.Edi), 8, 16, QLatin1Char('0')));
    setResWidgetTable(EIPROW,0,QString("0x%1").arg(uint(context.Eip), 8, 16, QLatin1Char('0')));
    setResWidgetTable(EFLAGSROW,0,QString("0x%1").arg(uint(context.EFlags), 8, 16, QLatin1Char('0')));
    setResFlag(context.EFlags);
}

void MainWindow::sltDisAssemblyData(unsigned int nAddress, QString qstrHex, QString qstrData)
{
    int nRow = ui->tableWidget_Disassembly->rowCount();
    if(nRow > 15)
    {
        return;
    }

    // 转换相对地址
    TransformRelAddress(nAddress,qstrHex.mid(0,2),qstrData);

    ui->tableWidget_Disassembly->insertRow(nRow);
    ui->tableWidget_Disassembly->setItem(nRow,0,new QTableWidgetItem(QString("0x%1").arg(uint(nAddress), 8, 16, QLatin1Char('0'))));
    ui->tableWidget_Disassembly->setItem(nRow,1,new QTableWidgetItem(qstrHex));
    ui->tableWidget_Disassembly->setItem(nRow,2,new QTableWidgetItem(qstrData));

    QString Temp;
    if(FindFucName(nAddress, Temp))
    {
        ui->tableWidget_Disassembly->setItem(nRow,0,new QTableWidgetItem(Temp));
    }

    if(qstrData.contains("call"))
    {
        auto qstrlist = qstrData.split('x');
        if(FindFucName(qstrlist.at(1).toUInt(nullptr,16), Temp))
        {
            ui->tableWidget_Disassembly->setItem(nRow,3,new QTableWidgetItem(Temp));
        }

    }

    //标记断点
    SignBreakPoint();
}

void MainWindow::sltMemInfo(QByteArray qbtaMem, unsigned int nLength, unsigned int nAddress)
{
    QString qstrHex;
    QString qstrAddress;
    // 可能不是16的倍数 单独处理
    int nRemain = nLength % 16;
    // 输出几行 1行16个
    nLength = nLength / 16;

    qstrHex = qbtaMem.toHex(' ');
    int i = 0;
    for(; i < nLength > 0; ++i)
    {
        ui->tableWidget_Memory->insertRow(i);
        ui->tableWidget_Memory->setItem(i,0,new QTableWidgetItem(QString::number(nAddress, 16)));
        ui->tableWidget_Memory->setItem(i,1,new QTableWidgetItem(qstrHex.mid(0,48)));
        nAddress += 16;
        qstrHex.remove(0, 48);
    }

    if(nRemain > 0)
    {
        ui->tableWidget_Memory->insertRow(i);
        ui->tableWidget_Memory->setItem(i,0,new QTableWidgetItem(QString::number(nAddress, 16)));
        ui->tableWidget_Memory->setItem(i,1,new QTableWidgetItem(qstrHex.mid(0,nRemain * 3)));
    }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch (index)
    {
    case UI_BPTABEL:
        BreakPointTable();
        break;
    case UI_DLL:
        LoadDllTable();
        break;
    }
}
