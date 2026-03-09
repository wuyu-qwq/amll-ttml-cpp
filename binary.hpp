#pragma once

#include <bitset>
#include <fstream>

#define MAX_SIZE 0xFFFFFFFF

class myBits {

public:
    void write(std::string& data) {
        for (size_t i = 0; i < data.size(); ++i) {
            uint8_t byte = static_cast<uint8_t>(data[i]);
            for (int j = 0; j < 8; ++j) {
                bool bit = (byte >> (7 - j)) & 1;
                bs.set(cursor + i * 8 + j, bit);
            }
        }
    }

private:
    std::bitset<MAX_SIZE> bs;
    unsigned long cursor;

};