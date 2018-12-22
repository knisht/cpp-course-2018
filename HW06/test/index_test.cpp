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
namespace
{
std::ifstream::pos_type file_size(const char *filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

struct ContextHandler {
    size_t getTransactionalId() { return 0; }
    template <typename... Args>
    void nothing(Args...)
    {
    }
    std::vector<QString> collector;
    void collect(QString const &filename, size_t)
    {
        collector.push_back(filename);
    }
} static contextHandler;

static TaskContext qsizetypeContext = TaskContext<ContextHandler, qsizetype>{
    0, &contextHandler, &ContextHandler::nothing<qsizetype>};

static TaskContext substirngContext =
    TaskContext<ContextHandler, QString const &>{
        0, &contextHandler, &ContextHandler::nothing<QString const &>};

void setUpIndex(TrigramIndex &index, std::string directory)
{
    std::vector<Document> filenames = TrigramIndex::getFileEntries(
        QString::fromStdString(directory), substirngContext);
    for (Document &doc : filenames) {
        TrigramIndex::unwrapTrigrams(doc, qsizetypeContext);
    }
    index.getFilteredDocuments(std::move(filenames), qsizetypeContext);
}

std::vector<QString> getRelevantFiles(TrigramIndex &index, std::string target)
{
    std::vector<QString> filenames =
        index.getCandidateFileNames(target, qsizetypeContext);
    ContextHandler catchHandler;
    TaskContext catcherContext =
        TaskContext<ContextHandler, QString const &, size_t>{
            0, &catchHandler, &ContextHandler::collect};

    index.findOccurrencesInFiles(filenames, target, catcherContext);
    return catchHandler.collector;
}

std::vector<size_t> positions(TrigramIndex &index, QString const &filename,
                              std::string target)
{
    ContextHandler catchHandler;
    TaskContext catcherContext =
        TaskContext<ContextHandler, QString const &, size_t>{
            0, &catchHandler, &ContextHandler::collect};
    return index.collectAllOccurrences(filename, target, catcherContext);
}

template <typename T>
void resort(std::vector<T> &a, std::vector<T> &b)
{
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
}

void rescan(TrigramIndex &index, QString const &directory)
{
    for (QString const &newFilename :
         index.reprocessDirectory(directory, qsizetypeContext)) {
        if (QFileInfo(newFilename).isDir()) {
            rescan(index, newFilename);
        }
    }
}

void reparse(TrigramIndex &index, QString const &file)
{
    index.reprocessFile(file, qsizetypeContext);
}

} // namespace

TEST(correctness_index, stable)
{
    std::string directory = "./tmp/";
    fs::create_directory(directory);
    std::ofstream(directory + "a") << "green" << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    fs::remove_all(directory);
}

TEST(correctness_index, basic_finding)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::ofstream(directory + "/a") << "green" << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<QString> occurrences = getRelevantFiles(index, "green");
    std::vector<QString> expected{QFileInfo("./tmp/a").absoluteFilePath()};
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

    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<QString> occurrences = getRelevantFiles(index, "aba");
    std::vector<QString> expected{QFileInfo("./tmp/a").absoluteFilePath(),
                                  QFileInfo("./tmp/b").absoluteFilePath()};
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
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<QString> occurrences = getRelevantFiles(index, "kek");
    std::vector<QString> expected{};
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
    setUpIndex(index, directory);
    std::vector<QString> occurrences = getRelevantFiles(index, "a");
    std::vector<QString> expected{QFileInfo("./tmp/a").absoluteFilePath(),
                                  QFileInfo("./tmp/b").absoluteFilePath()};
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
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<QString> occurrences = getRelevantFiles(index, "it ");
    std::vector<QString> expected{QFileInfo("./tmp/wow/a").absoluteFilePath(),
                                  QFileInfo("./tmp/wow/d").absoluteFilePath()};
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
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<QString> occurrences = getRelevantFiles(index, "aabbaa");
    std::vector<QString> expected{QFileInfo("./tmp/a").absoluteFilePath(),
                                  QFileInfo("./tmp/b").absoluteFilePath()};
    resort(occurrences, expected);
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_index, basic_binary)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target = "aaabbaa";
    target += '\0';
    target += "asdasd";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    ASSERT_EQ(index.getDocuments().size(), 0);
    fs::remove_all(directory);
}

TEST(correctness_index, garbage)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target;
    for (size_t i = 0; i < 10000; ++i) {
        target.push_back(rand() % 255);
    }
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    ASSERT_EQ(index.getDocuments().size(), 0);
    fs::remove_all(directory);
}

TEST(correctness_index, large_ascii)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target;
    for (size_t i = 0; i < 10000; ++i) {
        target.push_back((rand() % (127 - 32)) + 32);
    }
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    ASSERT_EQ(index.getDocuments().size(), 1);
    fs::remove_all(directory);
}

TEST(correctness_index, large_ascii_detect)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target;
    for (size_t i = 0; i < 10000; ++i) {
        target.push_back((rand() % (127 - 32)) + 32);
    }
    target += "friendly fire!";
    for (size_t i = 0; i < 10000; ++i) {
        target.push_back((rand() % (127 - 32)) + 32);
    }
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<QString> occurrences =
        getRelevantFiles(index, "friendly fire!");
    std::vector<QString> expected{QFileInfo("./tmp/a").absoluteFilePath()};
    ASSERT_EQ(occurrences, expected);
    fs::remove_all(directory);
}

TEST(correctness_finding, basic)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target = "The quick brown fox jumps over the lazy dog";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<size_t> occurrences =
        positions(index, QString::fromStdString(directory + "/a"), "lazy");
    std::vector<size_t> expected{35};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_finding, multiple)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target = "the quick brown fox jumps over the lazy dog";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<size_t> occurrences =
        positions(index, QString::fromStdString(directory + "/a"), "the");
    std::vector<size_t> expected{0, 31};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_finding, absense)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target = "the quick brown fox jumps over the lazy dog";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<size_t> occurrences =
        positions(index, QString::fromStdString(directory + "/a"), "c++");
    std::vector<size_t> expected{};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_finding, overlapping)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target = "aaabbbaaabbbaaabbbaaa";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<size_t> occurrences =
        positions(index, QString::fromStdString(directory + "/a"), "aaabbbaaa");
    std::vector<size_t> expected{0, 6, 12};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_finding, unicode)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target = "Ехал Грека через реку, видит Грека - в реке рак";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<size_t> occurrences =
        positions(index, QString::fromStdString(directory + "/a"), "Грека");
    std::vector<size_t> expected{5, 29};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_finding, large)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target;
    for (size_t i = 0; i < 100000; ++i) {
        target.push_back((rand() % (127 - 32)) + 32);
    }
    target += "friendly fire!";
    for (size_t i = 0; i < 100000; ++i) {
        target.push_back((rand() % (127 - 32)) + 32);
    }
    target += "friendly fire!";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    std::vector<size_t> occurrences = positions(
        index, QString::fromStdString(directory + "/a"), "friendly fire!");
    std::vector<size_t> expected{100000, 200014};
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_watching, add_file)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target = "abcde";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    target = "xyzwvu";
    std::ofstream(directory + "/b") << target << std::flush;
    rescan(index,
           QFileInfo(QString::fromStdString(directory)).absoluteFilePath());
    std::vector<QString> occurrences = getRelevantFiles(index, "yzwvu");
    std::vector<QString> expected{QFileInfo("./tmp/b").absoluteFilePath()};
    resort(occurrences, expected);
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_watching, change_file)
{

    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target = "abcde";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    target = "xyzwvu";
    std::ofstream(directory + "/a") << target << std::flush;
    reparse(
        index,
        QFileInfo(QString::fromStdString(directory + "/a")).absoluteFilePath());
    std::vector<QString> occurrences = getRelevantFiles(index, "yzwvu");
    std::vector<QString> expected{QFileInfo("./tmp/a").absoluteFilePath()};
    resort(occurrences, expected);
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}

TEST(correctness_watching, remove_file)
{

    std::string directory = "./tmp";
    fs::create_directories(directory);
    std::string target = "abcde";
    std::ofstream(directory + "/a") << target << std::flush;
    TrigramIndex index;
    setUpIndex(index, directory);
    fs::remove(directory + "/a");
    reparse(
        index,
        QFileInfo(QString::fromStdString(directory + "/a")).absoluteFilePath());
    ASSERT_EQ(static_cast<int>(0), index.getDocuments().size());
    fs::remove_all(directory);
}

TEST(correctness_watching, add_file_and_directory)
{
    std::string directory = "./tmp";
    fs::create_directories(directory);
    TrigramIndex index;
    setUpIndex(index, directory);
    fs::create_directory(directory + "/deeptmp");
    std::string target = "abcde";
    std::ofstream(directory + "/deeptmp/a") << target << std::flush;
    rescan(index,
           QFileInfo(QString::fromStdString(directory)).absoluteFilePath());
    std::vector<QString> occurrences = getRelevantFiles(index, "abcd");
    std::vector<QString> expected{
        QFileInfo("./tmp/deeptmp/a").absoluteFilePath()};
    resort(occurrences, expected);
    ASSERT_EQ(expected, occurrences);
    fs::remove_all(directory);
}
