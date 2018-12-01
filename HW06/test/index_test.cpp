#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <test/gtest/gtest.h>
#include <utility>
#include <vector>

#include "include/TrigramIndex.h"
#include <QString>

namespace fs = std::filesystem;

using SubstringOccurrence = TrigramIndex::SubstringOccurrence;

std::ifstream::pos_type file_size(const char *filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

TEST(correctness, stable)
{
    std::string directory = "./tmp/";
    fs::create_directory(directory);
    std::ofstream(directory + "a") << "green" << std::flush;
    TrigramIndex index{"./tmp"};
    fs::remove_all(directory);
}

TEST(correctness, basic_finding)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "green" << std::flush;
    TrigramIndex index{"./tmp"};

    std::vector<SubstringOccurrence> occurrences = index.findSubstring("green");
    std::vector<SubstringOccurrence> expected{{"./tmp/a", {0}}};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness, multiple_finding)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "abacaba" << std::flush;
    std::ofstream(directory + "/b") << "dabacaba" << std::flush;
    TrigramIndex index{"./tmp"};

    std::vector<SubstringOccurrence> occurrences = index.findSubstring("aba");
    std::vector<SubstringOccurrence> expected{{"./tmp/a", {0, 4}},
                                              {"./tmp/b", {1, 5}}};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness, absence)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "abacaba" << std::flush;
    std::ofstream(directory + "/b") << "dabacaba" << std::flush;
    TrigramIndex index{"./tmp"};

    std::vector<SubstringOccurrence> occurrences = index.findSubstring("kek");
    std::vector<SubstringOccurrence> expected{};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness, one_letter)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "abacaba" << std::flush;
    std::ofstream(directory + "/b") << "dabacaba" << std::flush;
    TrigramIndex index{"./tmp"};

    std::vector<SubstringOccurrence> occurrences = index.findSubstring("a");
    std::vector<SubstringOccurrence> expected{{"./tmp/a", {0, 2, 4, 6}},
                                              {"./tmp/b", {1, 3, 5, 7}}};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness, time)
{
    std::string directory = "/home/knisht/repos";
    //    fs::create_directories(directory);
    //    std::ofstream(directory + "/a") << "abacaba" << std::flush;
    //    std::ofstream(directory + "/b") << "dabacaba" << std::flush;
    TrigramIndex index{directory.c_str()};
    std::cout << "build index!" << std::endl;
    std::vector<SubstringOccurrence> occurrences =
        index.findSubstring("source");
    //    std::vector<SubstringOccurrence> expected{{"./tmp/a", {0, 2, 4, 6}},
    //                                              {"./tmp/b", {1, 3, 5, 7}}};
    //    ASSERT_EQ(expected, occurrences);
    //    fs::remove_all(directory);
}
