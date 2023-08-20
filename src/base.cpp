#include "json/base.h"

#include <cmath>

std::string JSON::Result::GetPrettyError(uint32_t viewRadius) const
{
    if (*this)
        return "";

    auto lineNumber = std::to_string(ErrorRow+1);
    auto lineCharNumber = std::to_string(ErrorColumn);

    uint32_t errorStart = ErrorColumn < viewRadius ?
        0 : ErrorColumn - viewRadius;
    uint32_t errorEnd = std::min<uint32_t>(ErrorLine.length(), ErrorColumn + viewRadius - 1);
    std::string trimmedLine = ErrorLine.substr(errorStart, errorEnd-errorStart);

    if (errorStart != 0)
        trimmedLine = "... " + trimmedLine;
    if (errorEnd != ErrorLine.length())
        trimmedLine += " ...";

    std::string error;
    error += std::string(lineNumber.length(), ' ') + " | " + Error + '\n';

    error += lineNumber + " | ";
    error += trimmedLine;
    error += '\n';

    std::string finder(ErrorColumn + (errorStart != 0 ? -errorStart + 4 : 0), ' ');
    if (finder.length() > 0)
        finder[finder.length()-1] = '^';
    else finder += '^';
    finder += '~' + lineCharNumber;
    error += std::string(lineNumber.length(), ' ') + " | ";
    error += finder;

    return error;

#if 0
    const char* viewStart = ErrorLineText.c_str() + ErrorLineChar-1 - std::min<uint32_t>(ErrorLineChar-1, viewRadius);
    const char* viewEnd = ErrorLineText.c_str() + ErrorLineChar-1 + std::min<uint32_t>(ErrorLineText.length()-ErrorLineChar-1, viewRadius);

    auto lineNumber = std::to_string(ErrorLine);
    auto lineCharNumber = std::to_string(ErrorLineChar);

    std::string errorLine(viewStart, viewEnd);
    for (size_t i = 0; i < errorLine.length(); i++) {
        // Remove any control character (CR-LF should not be present)
        JSON_ASSERT(errorLine[i] != '\r' && errorLine[i] != '\n');
        if (iscntrl((unsigned char)errorLine[i]))
            errorLine[i] = ' ';
    }

    std::string error;
    error += std::string(lineNumber.length(), ' ') + " | " + Error + '\n';

    error += lineNumber + " | ";
    if (viewStart == ErrorLineText.c_str())
        error += "... ";
    error += errorLine;
    if (viewEnd != ErrorLineText.c_str() + ErrorLineText.length() - 1)
        error += " ...";
    error += '\n';

    std::string finder(lineChar-trimmedFromError, ' ');
    finder += '^';
    finder += '~' + lineCharNumber;
    error += std::string(lineNumber.length(), ' ') + " | ";
    if (trimmedFromError > 0)
        error += "    ";
    error += finder;

    return error;
#endif
}
