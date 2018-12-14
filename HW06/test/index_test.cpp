#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <test/gtest/gtest.h>
#include <utility>
#include <vector>

#include <chrono>

#include "include/trigramindex.h"
#include <QString>

namespace fs = std::filesystem;

std::ifstream::pos_type file_size(const char *filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

struct ContextHandler {
    size_t getTransactionalId() { return 0; }
    void nothing(qsizetype) {}
    void nothing(QString const &) {}
} static contextHandler;

static TaskContext qsizetypeContext = TaskContext<ContextHandler, qsizetype>{
    0, &contextHandler, &ContextHandler::nothing};

static TaskContext substirngContext =
    TaskContext<ContextHandler, QString const &>{0, &contextHandler,
                                                 &ContextHandler::nothing};

std::vector<SubstringOccurrence> getOccurrences(std::string directory,
                                                QString const &pattern)
{
    TrigramIndex index;
    index.setUp(QString::fromStdString(directory), qsizetypeContext,
                substirngContext);
    return index.findSubstring(pattern, qsizetypeContext);
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

TEST(correctness_index, stable)
{
    std::string directory = "./tmp/";
    fs::create_directory(directory);
    std::ofstream(directory + "a") << "green" << std::flush;
    TrigramIndex index;
    fs::remove_all(directory);
}

TEST(correctness_index, basic_finding)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "green" << std::flush;
    std::vector<SubstringOccurrence> occurrences =
        getOccurrences(directory, "green");
    std::vector<SubstringOccurrence> expected{
        {QFileInfo("./tmp/a").absoluteFilePath(), {0}}};
    resort(occurrences, expected);
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_index, multiple_finding)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "abacaba" << std::flush;
    std::ofstream(directory + "/b") << "dabacaba" << std::flush;

    std::vector<SubstringOccurrence> occurrences =
        getOccurrences(directory, "aba");
    std::vector<SubstringOccurrence> expected{
        {QFileInfo("./tmp/a").absoluteFilePath(), {0, 4}},
        {QFileInfo("./tmp/b").absoluteFilePath(), {1, 5}}};
    resort(occurrences, expected);
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_index, absence)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "abacaba" << std::flush;
    std::ofstream(directory + "/b") << "dabacaba" << std::flush;

    std::vector<SubstringOccurrence> occurrences =
        getOccurrences(directory, "kek");
    std::vector<SubstringOccurrence> expected{};
    resort(occurrences, expected);
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_index, one_letter)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "abacaba" << std::flush;
    std::ofstream(directory + "/b") << "dabacaba" << std::flush;
    TrigramIndex index;
    std::vector<SubstringOccurrence> occurrences =
        getOccurrences(directory, "a");
    std::vector<SubstringOccurrence> expected{
        {QFileInfo("./tmp/a").absoluteFilePath(), {0, 2, 4, 6}},
        {QFileInfo("./tmp/b").absoluteFilePath(), {1, 3, 5, 7}}};
    resort(occurrences, expected);
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_index, deep)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "look at my horse" << std::flush;
    std::ofstream(directory + "/b") << "my horse is amazing" << std::flush;
    std::string subdirectory = directory + "/wow";
    fs::create_directories(subdirectory);
    std::ofstream(subdirectory + "/a") << "give it a lick" << std::flush;
    std::ofstream(subdirectory + "/d")
        << "mmm it tastes just like raisins" << std::flush;
    std::vector<SubstringOccurrence> occurrences =
        getOccurrences(directory, "it ");
    std::vector<SubstringOccurrence> expected{
        {QFileInfo("./tmp/wow/a").absoluteFilePath(), {5}},
        {QFileInfo("./tmp/wow/d").absoluteFilePath(), {4}}};
    resort(occurrences, expected);
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_index, overlapping_strings)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "aabbaabbaa" << std::flush;
    std::ofstream(directory + "/b") << "aabbcaabbaac" << std::flush;
    std::vector<SubstringOccurrence> occurrences =
        getOccurrences(directory, "aabbaa");
    std::vector<SubstringOccurrence> expected{
        {QFileInfo("./tmp/a").absoluteFilePath(), {0, 4}},
        {QFileInfo("./tmp/b").absoluteFilePath(), {5}}};
    resort(occurrences, expected);
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_finding, russian)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target = "Ехал Грека через реку, видит Грека - в реке рак";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    std::vector<SubstringOccurrence> occurrences =
        getOccurrences(directory, "Грека");
    std::vector<SubstringOccurrence> expected{
        {QFileInfo("./tmp/a").absoluteFilePath(), {5, 29}}};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

// TEST(correctness, time)
//{
//    std::string directory = "/home/knisht/repos";
//    //    std::string directory = ".";
//    //    fs::create_directories(directory);
//    //    std::ofstream(directory + "/a") << "abacaba" << std::flush;
//    //    std::ofstream(directory + "/b") << "dabacaba" << std::flush;
//    auto start = std::chrono::steady_clock::now();
//    TrigramIndex index;
//    index.setUp(QString::fromStdString(directory));
//    qDebug() << "indexed!" << endl;
//    //    index.setUp(directory.c_str());
//    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
//        std::chrono::steady_clock::now() - start);
//    std::cout << "KEK ->" << duration.count() << std::endl;
//    //    std::vector<SubstringOccurrence> occurrences =
//    //    index.findSubstring("index");
//    start = std::chrono::steady_clock::now();
//    duration = std::chrono::duration_cast<std::chrono::milliseconds>(
//        std::chrono::steady_clock::now() - start);
//    std::cout << "LOL ->" << duration.count() << std::endl;
//    //    std::vector<SubstringOccurrence> expected{{"./tmp/a", {0, 2, 4, 6}},
//    //                                              {"./tmp/b", {1, 3, 5,
//    //    7}}};
//    //    ASSERT_EQ(expected, occurrences);
//    //    fs::remove_all(directory);
//}
