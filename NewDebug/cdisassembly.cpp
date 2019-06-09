#include "cdisassembly.h"

CDisassembly::CDisassembly()
{

}

int CDisassembly::StartDisassembly(IN HANDLE hProcess,IN DWORD Eip, IN DWORD nLength, QMap<DWORD,BREAK_POINT> &pBpMap)
{
    unsigned char data[1024] = {0};

    SIZE_T bytesRead;
    if(!ReadProcessMemory(hProcess, (LPVOID)(Eip), data, nLength,&bytesRead))
    {
        OutputDebugStringA("ReadProcessMemory Error");
        OutputDebugStringA(QString::number(GetLastError()).toLocal8Bit());
        return FALSE;
    }

    // 修复自己下的断点 屏蔽cc以显示正确的反汇编
    for(auto it = pBpMap.begin(); it != pBpMap.end(); ++it)
    {
        if(it->address >= Eip &&  it->address <= Eip + nLength)
        {
            data[it->address - Eip] = it->content;
        }
    }

    // Initialize decoder context
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_ADDRESS_WIDTH_32);

    // Initialize formatter. Only required when you actually plan to do instruction
    // formatting ("disassembling"), like we do here
    ZydisFormatter formatter;
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    // Loop over the instructions in our buffer.
    // The runtime-address (instruction pointer) is chosen arbitrary here in order to better
    // visualize relative addressing
    ZyanU32 runtime_address = Eip;
    ZyanUSize offset = 0;
    const ZyanUSize length = sizeof(data);
    ZydisDecodedInstruction instruction;
    ZydisStatus status;
    size_t readOffs = 0;
    QByteArray qbyteHex;
    qbyteHex.resize(1024);
    memcpy_s(qbyteHex.data(),1024,data,1024);
    while ((status = ZydisDecoderDecodeBuffer(&decoder, data + offset, length - offset, readOffs, &instruction)
           != ZYDIS_STATUS_NO_MORE_DATA))
    {

        // Format & print the binary instruction structure to human readable format
        char buffer[256];

        ZydisFormatterFormatInstruction(&formatter, &instruction, buffer, sizeof(buffer));

        QString qstrDis = QString::fromLocal8Bit(buffer);
        QString qstrHex = qbyteHex.mid(offset,instruction.length).toHex(' ');

        emit sigDisAssemblyData(runtime_address, qstrHex ,qstrDis);

        offset += instruction.length;
        runtime_address += instruction.length;
    }

    return TRUE;
}
