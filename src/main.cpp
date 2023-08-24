#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_set>

#include "json/json.h"

std::ostream& operator<<(std::ostream& out, char32_t ch) { return out << JSON::UTF8::Encode(ch); }
std::ostream& operator<<(std::ostream& out, const char32_t* str) { return out << JSON::UTF8::Encode(str); }
std::ostream& operator<<(std::ostream& out, const std::u32string& str) { return out << JSON::UTF8::Encode(str); }

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

#if 0
    std::string json = "{ \"foo\": [ \"bar\", { \"euro\": \"\\u20AC\\u20AC\" }, true, null, false, 10, 20, 30e10, 22.4e-1 ] }";
    std::shared_ptr<JSON::Element> root;
    JSON::Result res = JSON::Parse(json, root, true, 255);
    std::cout << res.GetPrettyError(20) << std::endl;
    std::cout << root->At("foo")->At(1)->At("euro")->AsString().View.Replace(1, 3, "EEE").Ref << std::endl;

#else
    std::string json;
    std::filesystem::path path = "test/JSONTestSuite/test_parsing";

    std::unordered_set<std::string> filesToSkip { };

    size_t tests = 0;
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        auto fileName = entry.path().filename();
        if (filesToSkip.find(fileName.string()) != filesToSkip.end())
            continue;

        // i -> Doesn't matter if parsing succeedes or not
        // n -> Parsing must fail
        // y -> Parsing must succeed
        char s = (char)*fileName.c_str();
        ++tests;

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
        JSON::Result res = JSON::Parse(json, root, true, 128);

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
                std::cout << res.GetPrettyError(20) << std::endl;
                return 1;
            case 'i':
            case 'n':
                std::cout << entry.path() << " Was successful." << std::endl;
                std::cout << res.GetPrettyError(20) << std::endl;
                break;
            default:
                JSON_ASSERT(false);
            }
        }
    }

    std::cout << "Completed " << tests << " Tests." << std::endl;
#endif

    return 0;
}
