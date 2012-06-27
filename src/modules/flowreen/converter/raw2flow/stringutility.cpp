#include "stringutility.h"
#include <set>

std::vector<std::string> StringUtility::explode(const std::string& input, const char separator) {
    size_t lastPos = 0;
    std::vector<std::string> result;
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == separator) {
            result.push_back(input.substr(lastPos, (i - lastPos)));
            lastPos = i + 1;
        }
    }
    if (lastPos < input.length())
        result.push_back(input.substr(lastPos, (lastPos - input.length() - 1)));
    return result;
}

std::string StringUtility::getFileExtension(const std::string& filename) {
    size_t pos = filename.rfind('.');
    if (pos != std::string::npos)
        return filename.substr(pos + 1);
    return "";
}

bool StringUtility::isCharInSet(const char input, const std::string& options) {
    std::set<char> optionsSet;
    for (size_t i = 0; i < options.size(); ++i)
        optionsSet.insert(options[i]);
    return (optionsSet.find(input) != optionsSet.end());
}
