#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

#include "tinyxml2/tinyxml2.h"

struct CharInfo {
    uint32_t startTime;
    uint32_t endTime;
    std::string character;
    std::string roman;
};

struct Para {
    bool paraPos;    // 段落位置，true为居右
    bool bg = false; // 背景歌词，true为背景歌词
    uint32_t startTime;
    uint32_t endTime;
    std::string key;
    std::vector<CharInfo> lyric;
    std::string translation;
    std::string roman;
};

struct Song {
    std::vector<Para> lyrics;
    std::map<std::string, std::vector<std::string>> metadata;
};

// 时间字符串解析函数（增强版）
// 支持格式：hh:mm:ss.mmm, mm:ss.mmm, s.sss, s
static uint32_t parseTime(const char* timeStr) {
    if (!timeStr) return 0;
    std::string s = timeStr;
    // trim
    auto ltrim = [](std::string &str){
        size_t p = str.find_first_not_of(" \t\r\n");
        if (p == std::string::npos) str.clear();
        else if (p) str.erase(0, p);
    };
    auto rtrim = [](std::string &str){
        size_t p = str.find_last_not_of(" \t\r\n");
        if (p == std::string::npos) str.clear();
        else if (p+1 < str.size()) str.erase(p+1);
    };
    ltrim(s); rtrim(s);
    if (s.empty()) return 0;

    try {
        size_t colonCount = std::count(s.begin(), s.end(), ':');

        auto parseSecAndFrac = [](const std::string &part) -> std::pair<int,int> {
            // 返回 {seconds, milliseconds}
            size_t dot = part.find('.');
            if (dot == std::string::npos) {
                int sec = std::stoi(part);
                return { sec, 0 };
            } else {
                std::string secPart = part.substr(0, dot);
                std::string fracPart = part.substr(dot+1);
                int sec = secPart.empty() ? 0 : std::stoi(secPart);
                // 只取前 3 位小数并补齐
                if (fracPart.size() > 3) fracPart = fracPart.substr(0,3);
                while (fracPart.size() < 3) fracPart.push_back('0');
                int ms = std::stoi(fracPart);
                return { sec, ms };
            }
        };

        if (colonCount == 2) {
            // hh:mm:ss(.mmm)
            size_t p1 = s.find(':');
            size_t p2 = s.find(':', p1+1);
            std::string hStr = s.substr(0, p1);
            std::string mStr = s.substr(p1+1, p2-p1-1);
            std::string rest = s.substr(p2+1);
            int h = hStr.empty() ? 0 : std::stoi(hStr);
            int m = mStr.empty() ? 0 : std::stoi(mStr);
            auto [sec, ms] = parseSecAndFrac(rest);
            long totalMs = (long)h*3600*1000 + (long)m*60*1000 + (long)sec*1000 + ms;
            return (int)totalMs;
        } else if (colonCount == 1) {
            // mm:ss(.mmm)
            size_t p = s.find(':');
            std::string mStr = s.substr(0, p);
            std::string rest = s.substr(p+1);
            int m = mStr.empty() ? 0 : std::stoi(mStr);
            auto [sec, ms] = parseSecAndFrac(rest);
            long totalMs = (long)m*60*1000 + (long)sec*1000 + ms;
            return (int)totalMs;
        } else {
            // seconds or seconds.fraction
            // 使用 stod 以支持小数秒
            double secf = std::stod(s);
            if (secf < 0) return 0;
            long totalMs = (long)std::llround(secf * 1000.0);
            return (int)totalMs;
        }
    } catch (...) {
        return 0; // 解析失败返回0
    }
}

Song prase(std::string xmlContent) {
    Song result;

    // 处理空格
    size_t subpos = xmlContent.find("</span> ");
    while (subpos != std::string::npos) {
    	xmlContent.replace(subpos, 8, " </span>");
    	subpos = xmlContent.find("</span> ", subpos+8);
	}

    // 解析正文XML结构
    tinyxml2::XMLDocument doc;
    if (doc.Parse(xmlContent.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "XML解析失败! 错误代码: " << doc.ErrorID() << std::endl;
        return Song();
    }
    tinyxml2::XMLElement* tt = doc.FirstChildElement("tt");
    if (!tt) { std::cerr << "格式错误: 缺少 <tt>" << std::endl; return Song(); }
    tinyxml2::XMLElement* div = tt->FirstChildElement("body")->FirstChildElement("div");
    if (!div) { std::cerr << "格式错误: 缺少 <body>或<div>" << std::endl; return Song(); }

    // 将正文XML数据提取到数据结构
    for (tinyxml2::XMLElement* p = div->FirstChildElement("p"); p; p = p->NextSiblingElement("p")) {
        Para singlePara;
        Para bgPara;

        const char* paraKey = p->Attribute("itunes:key");
        const char* paraPos = p->Attribute("ttm:agent");
        const char* beginAttr = p->Attribute("begin");
        const char* endAttr = p->Attribute("end");
        singlePara.key = paraKey;
        singlePara.paraPos = strcmp(paraPos, "v2");
        singlePara.startTime = parseTime(beginAttr);
        singlePara.endTime = parseTime(endAttr);

        for (tinyxml2::XMLElement* span = p->FirstChildElement("span"); span;
        span = span->NextSiblingElement("span")) {
            const char* roleAttr  = span->Attribute("ttm:role");
            const char* text      = span->GetText();
            const char* beginAttr = span->Attribute("begin");
            const char* endAttr   = span->Attribute("end");

            if (!roleAttr)
                singlePara.lyric.push_back(CharInfo{parseTime(beginAttr), parseTime(endAttr), text});
            else if (strcmp(roleAttr, "x-translation") == 0) singlePara.translation = text;
            else if (strcmp(roleAttr, "x-roman") == 0) singlePara.roman = text;
            else if (strcmp(roleAttr, "x-bg") == 0) {
                bgPara.startTime = parseTime(beginAttr);
                bgPara.endTime   = parseTime(endAttr);
                bgPara.bg        = true;
                bgPara.paraPos   = singlePara.paraPos;
                for (tinyxml2::XMLElement* bgspan = span->FirstChildElement("span"); bgspan;
                bgspan = bgspan->NextSiblingElement("span")) {
                    const char* roleAttr = bgspan->Attribute("ttm:role");
                    const char* text     = bgspan->GetText();
                    if (!roleAttr) {
                        const char* beginAttr = bgspan->Attribute("begin");
                        const char* endAttr   = bgspan->Attribute("end");
                        bgPara.lyric.push_back(CharInfo{parseTime(beginAttr), parseTime(endAttr), text});
                    } else if (strcmp(roleAttr, "x-translation") == 0) bgPara.translation = text;
                    else if (strcmp(roleAttr, "x-roman") == 0) bgPara.roman = text;
                }
            }
        }
        result.lyrics.push_back(singlePara);
        if (!bgPara.lyric.empty()) result.lyrics.push_back(bgPara);
    }
    if (result.lyrics.empty()) {
        std::cout << "未解析到有效歌词数据！" << std::endl;
        return Song();
    }

    // 解析头部XML结构
    tinyxml2::XMLElement* metadata = tt->FirstChildElement("head")->FirstChildElement("metadata");
    if (!metadata) { std::cerr << "格式错误: 缺少 <head>或<metadata>" << std::endl; return Song(); }

    // 提取头部元数据
    tinyxml2::XMLElement* meta = metadata->FirstChildElement("amll:meta");
    for (tinyxml2::XMLElement* meta = metadata->FirstChildElement("amll:meta"); meta;
    meta = meta->NextSiblingElement("amll:meta")) 
        result.metadata[meta->Attribute("key")].push_back(meta->Attribute("value"));

    // 提取逐字音译数据
    tinyxml2::XMLElement* translation = metadata->FirstChildElement("iTunesMetadata");
	translation = translation ? translation->FirstChildElement("transliterations") : nullptr;
    if (translation) {
		translation = translation->FirstChildElement("transliteration");
		std::map<std::string, std::vector<std::string>> romans;
		for (tinyxml2::XMLElement* trans = translation->FirstChildElement("text"); trans;
		trans = trans->NextSiblingElement("text")) {
			std::string key = trans->Attribute("for");
			for (tinyxml2::XMLElement* span = trans->FirstChildElement("span"); span;
			span = span->NextSiblingElement("span"))
				romans[key].push_back(span->GetText());
		}
		for (Para& para : result.lyrics) {
			if (romans[para.key].empty()) continue;
			for (uint16_t i = 0; i < para.lyric.size(); ++i)
				para.lyric[i].roman = romans[para.key][i];
		}
	}

    return result;
}