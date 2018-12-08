#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <test/gtest/gtest.h>
#include <utility>
#include <vector>

#include <chrono>

#include "include/TrigramIndex.h"
#include <QString>

namespace fs = std::filesystem;

std::ifstream::pos_type file_size(const char *filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

void resort(std::vector<SubstringOccurrence> &a,
            std::vector<SubstringOccurrence> &b)
{
    sort(a.begin(), a.end(),
         [](SubstringOccurrence const &a, SubstringOccurrence const &b) {
             return a.filename < b.filename;
         });
    sort(b.begin(), b.end(),
         [](SubstringOccurrence const &a, SubstringOccurrence const &b) {
             return a.filename < b.filename;
         });
}

// TEST(correctness, stable)
//{
//    std::string directory = "./tmp/";
//    fs::create_directory(directory);
//    std::ofstream(directory + "a") << "green" << std::flush;
//    TrigramIndex index;
//    //    index.setUp("./tmp");
//    fs::remove_all(directory);
//}

// TEST(correctness, basic_finding)
//{
//    std::string directory = "./tmp";
//    fs::create_directories(directory);
//    std::ofstream(directory + "/a") << "green" << std::flush;
//    TrigramIndex index;
//    index.setUp(QString::fromStdString(directory));
//    std::vector<SubstringOccurrence> occurrences =
//    index.findSubstring("green"); std::vector<SubstringOccurrence>
//    expected{{"./tmp/a", {0}}}; resort(occurrences, expected);
//    ASSERT_EQ(expected, occurrences);
//    fs::remove_all(directory);
//}

// TEST(correctness, multiple_finding)
//{
//    std::string directory = "./tmp";
//    fs::create_directories(directory);
//    std::ofstream(directory + "/a") << "abacaba" << std::flush;
//    std::ofstream(directory + "/b") << "dabacaba" << std::flush;
//    TrigramIndex index;
//    index.setUp(QString::fromStdString(directory));
//    //    index.setUp("./tmp");

//    std::vector<SubstringOccurrence> occurrences = index.findSubstring("aba");
//    std::vector<SubstringOccurrence> expected{{"./tmp/a", {0, 4}},
//                                              {"./tmp/b", {1, 5}}};
//    resort(occurrences, expected);
//    ASSERT_EQ(expected, occurrences);
//    fs::remove_all(directory);
//}

// TEST(correctness, absence)
//{
//    std::string directory = "./tmp";
//    fs::create_directories(directory);
//    std::ofstream(directory + "/a") << "abacaba" << std::flush;
//    std::ofstream(directory + "/b") << "dabacaba" << std::flush;
//    TrigramIndex index;
//    index.setUp(QString::fromStdString(directory));
//    //    index.setUp("./tmp");

//    std::vector<SubstringOccurrence> occurrences = index.findSubstring("kek");
//    std::vector<SubstringOccurrence> expected{};
//    resort(occurrences, expected);
//    ASSERT_EQ(expected, occurrences);
//    fs::remove_all(directory);
//}

// TEST(correctness, one_letter)
//{
//    std::string directory = "./tmp";
//    fs::create_directories(directory);
//    std::ofstream(directory + "/a") << "abacaba" << std::flush;
//    std::ofstream(directory + "/b") << "dabacaba" << std::flush;
//    TrigramIndex index;
//    index.setUp(QString::fromStdString(directory));
//    //    index.setUp("./tmp");

//    std::vector<SubstringOccurrence> occurrences = index.findSubstring("a");
//    std::vector<SubstringOccurrence> expected{{"./tmp/a", {0, 2, 4, 6}},
//                                              {"./tmp/b", {1, 3, 5, 7}}};
//    resort(occurrences, expected);
//    ASSERT_EQ(expected, occurrences);
//    fs::remove_all(directory);
//}

// TEST(correctness, deep)
//{
//    std::string directory = "./tmp";
//    fs::create_directories(directory);
//    std::ofstream(directory + "/a") << "look at my horse" << std::flush;
//    std::ofstream(directory + "/b") << "my horse is amazing" << std::flush;
//    std::string subdirectory = directory + "/wow";
//    fs::create_directories(subdirectory);
//    std::ofstream(subdirectory + "/a") << "give it a lick" << std::flush;
//    std::ofstream(subdirectory + "/d")
//        << "mmm it tastes just like raisins" << std::flush;
//    TrigramIndex index;
//    index.setUp(QString::fromStdString(directory));
//    //    index.setUp("./tmp");

//    std::vector<SubstringOccurrence> occurrences = index.findSubstring("it ");
//    std::vector<SubstringOccurrence> expected{{"./tmp/wow/a", {5}},
//                                              {"./tmp/wow/d", {4}}};
//    resort(occurrences, expected);
//    ASSERT_EQ(expected, occurrences);
//    fs::remove_all(directory);
//}

// TEST(correctness, overlapping_strings)
//{
//    std::string directory = "./tmp";
//    fs::create_directories(directory);
//    std::ofstream(directory + "/a") << "aabbaabbaa" << std::flush;
//    std::ofstream(directory + "/b") << "aabbcaabbaac" << std::flush;
//    TrigramIndex index;
//    index.setUp(QString::fromStdString(directory));
//    //    index.setUp("./tmp");

//    std::vector<SubstringOccurrence> occurrences =
//        index.findSubstring("aabbaa");
//    std::vector<SubstringOccurrence> expected{{"./tmp/a", {0, 4}},
//                                              {"./tmp/b", {5}}};
//    resort(occurrences, expected);
//    ASSERT_EQ(expected, occurrences);
//    fs::remove_all(directory);
//}

TEST(correctness, time)
{
    std::string directory = "/home/knisht/repos/";
    //    std::string directory = ".";
    //    fs::create_directories(directory);
    //    std::ofstream(directory + "/a") << "abacaba" << std::flush;
    //    std::ofstream(directory + "/b") << "dabacaba" << std::flush;
    auto start = std::chrono::steady_clock::now();
    TrigramIndex index;
    index.setUp(QString::fromStdString(directory));
    //    index.setUp(directory.c_str());
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    std::cout << duration.count() << std::endl;
    std::vector<SubstringOccurrence> occurrences = index.findSubstring("index");
    start = std::chrono::steady_clock::now();
    int a = 0;
    for (int i = 0; i < 1000000; ++i) {
        ++a;
    }
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    std::cout << "LOL ->" << duration.count() << std::endl;
    //    std::vector<SubstringOccurrence> expected{{"./tmp/a", {0, 2, 4, 6}},
    //                                              {"./tmp/b", {1, 3, 5,
    //    7}}};
    //    ASSERT_EQ(expected, occurrences);
    //    fs::remove_all(directory);
}
