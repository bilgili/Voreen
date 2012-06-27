#ifndef STRINGUTILITY_H
#define STRINGUTILITY_H

#include <string>
#include <vector>

class StringUtility {
public:
    static std::vector<std::string> explode(const std::string& input, const char separator);
    static std::string getFileExtension(const std::string& filename);
    static bool isCharInSet(const char input, const std::string& options);
};

#endif
