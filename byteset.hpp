#pragma once

#include <vector>
#include <fstream>

class ByteSet {
    private:
    std::vector<uint8_t> data;

    public:
    ByteSet() = default;

    ByteSet(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) throw std::runtime_error("无法打开文件: " +
            filename);
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        data.resize(size);
        file.read((char*)data.data(), size);
        file.close();
    }

    void set(size_t index, uint8_t value) { data[index] = value; }

    uint8_t get(size_t index) const { return data[index]; }

    void write(uint8_t value) { data.push_back(value); }

    void write(std::string value) { data.insert(data.end(), value.begin(), value.end()); }

    void writeVarint(uint64_t value) {
        while (value >= 0x80) {
            data.push_back((value & 0x7F) | 0x80);
            value >>= 7;
        }
        data.push_back(value & 0x7F);
    }

    uint64_t readVarint(size_t index) const {
        uint64_t result = 0;
        int shift = 0;
        while (true) {
            uint8_t byte = data[index++];
            result |= (uint64_t)(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) break;
            shift += 7;
        }
        return result;
    }

    void outToFile(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);
        file.write((const char*)data.data(), data.size());
        file.close();
    }
};