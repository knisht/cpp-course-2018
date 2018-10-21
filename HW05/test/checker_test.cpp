#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <test/gtest/gtest.h>
#include <utility>
#include <vector>

#include "../include/checker.hpp"

namespace fs = std::filesystem;

typedef std::vector<std::vector<std::string>> groups;

TEST(correctness, basic)
{
    // Correctness of the ability to do something useful
    std::string directory = "./tmp/";
    fs::create_directory(directory);
    std::ofstream(directory + "a").write("green", sizeof("green"));
    groups res = core::group_all(directory);
    fs::remove_all(directory);
    ASSERT_EQ(res, groups{{directory + "a"}});
}

TEST(correctness, two_inequal_files)
{
    // Correctness of ability to detect different files
    std::string directory = "./tmp/";
    fs::create_directory(directory);
    std::string content1 = "green";
    std::string content2 = "red";
    std::ofstream(directory + "a")
        .write(content1.c_str(), sizeof(content1.c_str()));
    std::ofstream(directory + "b")
        .write(content2.c_str(), sizeof(content2.c_str()));
    groups actual = core::group_all(directory);
    fs::remove_all(directory);
    groups expected{{directory + "a"}, {directory + "b"}};
    ASSERT_EQ(expected, actual);
}

TEST(correctness, two_equal_files)
{
    // Correctness of ability to detect two equal files
    std::string directory = "./tmp/";
    fs::create_directory(directory);
    std::string content1 = "green";
    std::ofstream(directory + "a")
        .write(content1.c_str(), sizeof(content1.c_str()));
    std::ofstream(directory + "b")
        .write(content1.c_str(), sizeof(content1.c_str()));
    groups actual = core::group_all(directory);
    fs::remove_all(directory);
    groups expected{{directory + "a", directory + "b"}};
    ASSERT_EQ(expected, actual);
}

TEST(correctness, mixup)
{
    // Mixup of two previous approaches
    std::string directory = "./tmp/";
    fs::create_directory(directory);
    std::vector<std::pair<std::string, std::string>> files_content{
        {"a", "abcde"}, {"b", "cdef"}, {"c", "abcde"},
        {"d", "eghi"},  {"e", "cdef"}, {"h", "abcde"}};
    for (auto &&it : files_content) {
        std::ofstream(directory + it.first)
            .write(it.second.c_str(), sizeof(it.second.c_str()));
    }
    groups actual = core::group_all(directory);
    std::for_each(actual.begin(), actual.end(),
                  [](std::vector<std::string> &target) {
                      std::sort(target.begin(), target.end());
                  });
    fs::remove_all(directory);
    groups expected{{directory + "a", directory + "c", directory + "h"},
                    {directory + "b", directory + "e"},
                    {directory + "d"}};
    ASSERT_EQ(expected, actual);
}

TEST(correctness, mixup_big)
{
    // Ability to work with large files
    std::string directory = "./tmp/";
    fs::create_directory(directory);
    std::vector<std::string> files_content{"a", "b", "c", "d", "e",
                                           "f", "g", "h", "i"};
    std::string content1;
    for (int i = 0; i < 100000; ++i) {
        content1 += static_cast<char>(rand());
        content1 += static_cast<char>(rand());
        content1 += static_cast<char>(rand());
        content1 += static_cast<char>(rand());
        content1 += static_cast<char>(rand());
    }
    std::string content2;
    for (int i = 0; i < 100000; ++i) {
        content1 += static_cast<char>(rand());
        content1 += static_cast<char>(rand());
        content1 += static_cast<char>(rand());
        content1 += static_cast<char>(rand());
        content1 += static_cast<char>(rand());
    }
    groups expected{{}, {}};
    for (auto const &it : files_content) {
        bool check;
        if (rand() % 2 == 0) {
            expected[0].push_back(directory + it);
            check = false;
        } else {
            expected[1].push_back(directory + it);
            check = true;
        }
        std::string &content = check ? content1 : content2;
        std::ofstream(directory + it)
            .write(content.c_str(), sizeof(content.c_str()));
    }
    groups actual = core::group_all(directory);
    std::for_each(actual.begin(), actual.end(),
                  [](std::vector<std::string> &target) {
                      std::sort(target.begin(), target.end());
                  });
    auto comparator = [](std::vector<std::string> const &a,
                         std::vector<std::string> const &b) {
        for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
            if (a[i] != b[i]) {
                return a[i] < b[i];
            }
        }
        return a.size() < b.size();
    };
    std::sort(actual.begin(), actual.end(), comparator);
    std::sort(expected.begin(), expected.end(), comparator);
    fs::remove_all(directory);
    ASSERT_EQ(expected, actual);
}
