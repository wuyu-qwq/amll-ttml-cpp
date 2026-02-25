#include <fstream>
#include <chrono>

#include "praser.hpp"

unsigned int getTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int main() {
    // 读取文件内容
    std::ifstream file("./test.ttml", std::ios::binary);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string xmlContent(size, '\0');
    if (!file.is_open()) xmlContent = "";
    file.read(&xmlContent[0], size);
    file.close();
    if (xmlContent.empty()) {
        std::cerr << "文件内容为空或读取失败" << std::endl;
        return 1;
    }

    unsigned int time1 = getTime();
    std::vector<Para> lyrics = prase(xmlContent);
    unsigned int time2 = getTime();
    for (Para& para_iter : lyrics) {
        std::cout << "段落翻译：" << para_iter.translation << std::endl;
        std::cout << "段落罗马音：" << para_iter.roman << std::endl;
        std::cout << "是否背景歌词：" << (para_iter.bg ? "是" : "否") << std::endl;
        for (CharInfo& char_iter : para_iter.lyric) {
            std::cout << "歌词：" << char_iter.character << std::endl;
            std::cout << "罗马音：" << char_iter.roman << std::endl;
        }
        std::cout << "--------------------------------" << std::endl;
    }
    std::cout << "解析完成！用时: " << time2-time1 << "ms" << std::endl;
    return 0;
}