#pragma once

#include <vector>
#include <fstream>

class ByteSet {
    private:
    std::vector<uint8_t> data;

    public:
    void set(size_t index, uint8_t value) { data[index] = value; }

    uint8_t get(size_t index) const { return data[index]; }

    void outToFile(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);
        file.write((const char*)data.data(), data.size());
        file.close();
    }

    // template<typename T>
    // void append(const T& value) {
    //     const uint8_t* bytes = (const uint8_t*)&value;
    //     data.insert(data.end(), bytes, bytes + sizeof(T));
    // }

    void append(uint8_t value) { data.push_back(value); }

    void append(std::string value) { data.insert(data.end(), value.begin(), value.end()); }
};