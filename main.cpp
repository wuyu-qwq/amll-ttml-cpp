#pragma once

#include "praser.hpp"
#include "byteset.hpp"

#if defined(_WIN32)
#include <windows.h>
#endif

int main() {
    #if defined(_WIN32)
    SetConsoleOutputCP(65001);
    #endif    

    ByteSet byteSet;
    byteSet.write("AMLX"); // 写入Magic
    byteSet.write(0x01);   // 写入Version
    byteSet.write(0x00);   // 写入GlobalFlags（v1未启用）

    //byteSet.writeVarint(337);
    //std::cout << byteSet.readVarint(6);

    byteSet.outToFile("output.bin");

    return 0;
}