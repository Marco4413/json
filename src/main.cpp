#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_set>

#include "json/json.h"

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    std::string json;
    std::filesystem::path path = "test/JSONTestSuite/test_parsing";

    std::unordered_set<std::string> filesToSkip {
        // Parser expects '\0' as the end of file
        "n_multidigit_number_then_00.json",
        // Has unsupported unicode character
        "y_object_string_unicode.json",
        "y_string_1_2_3_bytes_UTF-8_sequences.json",
        "y_string_accepted_surrogate_pair.json",
        "y_string_accepted_surrogate_pairs.json",
        "y_string_escaped_noncharacter.json",
        "y_string_last_surrogates_1_and_2.json",
        "y_string_surrogates_U+1D11E_MUSICAL_SYMBOL_G_CLEF.json",
        "y_string_three-byte-utf-8.json",
        "y_string_two-byte-utf-8.json",
        "y_string_uEscape.json",
        "y_string_unicode.json",
        "y_string_unicode_U+10FFFE_nonchar.json",
        "y_string_unicode_U+1FFFE_nonchar.json",
        "y_string_unicode_U+200B_ZERO_WIDTH_SPACE.json",
        "y_string_unicode_U+2064_invisible_plus.json",
        "y_string_unicode_U+FDD0_nonchar.json",
        "y_string_unicode_U+FFFE_nonchar.json",
    };

    size_t tests = 0;
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        auto fileName = entry.path().filename();
        if (filesToSkip.find(fileName.string()) != filesToSkip.end())
            continue;
        ++tests;

        // i -> Doesn't matter if parsing succeedes or not
        // n -> Parsing must fail
        // y -> Parsing must succeed
        char s = (char)*fileName.c_str();

        std::ifstream file(entry.path());
        if (!file) {
            std::cout << entry.path() << " Could not be read." << std::endl;
            return 1;
        }

        std::streamsize fileSize = file.seekg(0, file.end).tellg();
        file.seekg(file.beg);

        json.resize(fileSize);
        file.read(json.data(), fileSize);
        file.close();

        std::shared_ptr<JSON::Element> root;
        JSON::Result res = JSON::Parse(json.data(), root, true, 128);

        if (res) {
            switch (s) {
            case 'i':
            case 'y':
                std::cout << entry.path() << " Was successful." << std::endl;
                break;
            case 'n':
                std::cout << entry.path() << " Failed." << std::endl;
                return 1;
            default:
                JSON_ASSERT(false);
            }
        } else {
            switch (s) {
            case 'y':
                std::cout << entry.path() << " Failed." << std::endl;
                std::cout << res.GetPrettyError(json.data(), 20) << std::endl;
                return 1;
            case 'i':
            case 'n':
                std::cout << entry.path() << " Was successful." << std::endl;
                std::cout << res.GetPrettyError(json.data(), 20) << std::endl;
                break;
            default:
                JSON_ASSERT(false);
            }
        }
    }

    std::cout << "Completed " << tests << " Tests." << std::endl;
    return 0;
}
