#include "include/TrigramIndex.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <algorithm>
#include <functional>
#include <iostream>
#include <unordered_set>
#include <utility>



void TrigramIndex::printDocuments()
{
    std::cout << "Files at all: " << documents.size() <<std::endl;
    for (auto &it : documents) {
        std::cout << it.filename.toStdString() << std::endl;
    }
}


const qint32 BUF_SIZE = 1 << 20;

std::vector<TrigramIndex::Document> getFileEntries(QString const &root);

void unwrapTrigrams(TrigramIndex::Document &document);

TrigramIndex::TrigramIndex() : valid(false) {}


void TrigramIndex::unwrapTrigrams(TrigramIndex::Document &document)
{
    QFile fileInstance{document.filename};
    // TODO: make error notifying
    fileInstance.open(QFile::ReadOnly);
    qint32 fileSize = static_cast<qint32>(fileInstance.size());
    // NOTE: files with filesize <= 2 are ignored
    if (fileSize <= 2) {
        return;
    }
    qint32 block_size = qMin(fileSize, BUF_SIZE);
    QByteArray bytes(block_size, '\0');
    char last[3];
    int passed = 0;
    while (fileSize > 0) {
        fileInstance.read(bytes.data(), block_size);

        bool has_zero = false;
#pragma omp parallel for
        for (int i = 0; i < block_size; ++i) {
            if (bytes.data()[i] == 0) {
                has_zero = true;
            }
        }
        if (has_zero) {
            document.trigramOccurrences.clear();
            return;
        }
        if (passed > 0) {
            last[2] = bytes[0];
            document.trigramOccurrences[{last}].push_back(
                static_cast<size_t>(passed * block_size - 2));
            char trigramBuf[3] = {last[1], last[2], bytes[1]};
            document.trigramOccurrences[{trigramBuf}].push_back(
                static_cast<size_t>(passed * block_size - 1));
        }

        for (size_t i = 0; i < qMin(fileSize, block_size) - 2; ++i) {
            size_t trigram_code =
                static_cast<size_t>(bytes.data()[i] << 16) +
                static_cast<size_t>(bytes.data()[i + 1] << 8) +
                static_cast<size_t>(bytes.data()[i + 2]);
            document.trigramOccurrences[trigram_code].push_back(
                passed * block_size + i);
        }
        if (document.trigramOccurrences.size() > 200000) {
            document.trigramOccurrences.clear();
            return;
        }
        last[0] = bytes[block_size - 2];
        last[1] = bytes[block_size - 1];
        fileSize -= block_size;
        block_size = qMin(block_size, fileSize);
        ++passed;
    }
    fileInstance.close();
}

//void TrigramIndex::mergeVectorToList(std::list<size_t> &destination,
//                       std::vector<size_t> const &source)
//{
//    auto it = destination.begin();
//    for (size_t currentIndex : source) {
//        while (it != destination.end() && *it < currentIndex) {
//            destination.erase(it++);
//        }
//        if (it == destination.end()) {
//            return;
//        }
//        if (*it == currentIndex) {
//            ++it;
//        }
//    }
//}

//std::vector<size_t> TrigramIndex::findExactOccurrences(Document const &doc,
//                                         std::string const &target)
//{
//    QFile fileInstance(doc.filename);
//    fileInstance.open(QFile::ReadOnly);
//    size_t currentPosition = 0;
//    TrigramIndex::Trigram start{target};
//    std::string buf(target.size(), '\0');
//    std::vector<size_t> result;
//    if (doc.trigramOccurrences.count(start) != 0) {
//        for (size_t occurrence : doc.trigramOccurrences.at(start)) {
//            if (occurrence < currentPosition) {
//                size_t delta = currentPosition - occurrence;
//                for (size_t i = 0; i < delta; ++i) {
//                    buf[i] = buf[i + target.size() - delta];
//                }
//                fileInstance.read(&buf[0] + delta,
//                                  static_cast<int>(target.size() - delta));
//            } else {
//                fileInstance.skip(
//                    static_cast<int>(occurrence - currentPosition));
//                fileInstance.read(&buf[0], static_cast<int>(target.size()));
//            }
//            if (strncmp(&buf[0], &target[0], target.size()) == 0) {
//                result.push_back(occurrence);
//            }
//            currentPosition = occurrence + target.size();
//        }
//    }
//    return result;
//}

//std::vector<SubstringOccurrence>
//TrigramIndex::smallStringProcess(std::string const &target) const
//{
//    std::unordered_set<size_t> fileIds;
//    for (auto &pair : trigramsInFiles) {
//        if (pair.first.substr(target)) {
//            fileIds.insert(pair.second.begin(), pair.second.end());
//        }
//    }
//    std::vector<SubstringOccurrence> result;
//    for (size_t fileId : fileIds) {
//        std::vector<size_t> occurrences;
//        for (auto &pair : documents[fileId].trigramOccurrences) {
//            if (pair.first.substr(target)) {
//                for (size_t j : pair.second) {
//                    occurrences.push_back(j);
//                }
//            }
//        }
//        // TODO: end of file trigrams
//        // TODO: move occurrs
//        result.push_back(
//            SubstringOccurrence{documents[fileId].filename, occurrences});
//    }
//    for (auto &it : result) {
//        std::sort(it.occurrences.begin(), it.occurrences.end());
//        it.occurrences.erase(
//            std::unique(it.occurrences.begin(), it.occurrences.end()),
//            it.occurrences.end());
//    }
//    return result;
//}

//std::vector<SubstringOccurrence>
//TrigramIndex::findSubstring(QString const &target) const
//{
//    std::string stdTarget = target.toStdString();
//    if (stdTarget.size() <= 2) {
//        return smallStringProcess(stdTarget);
//    }
//    std::unordered_set<TrigramIndex::Trigram, TrigramHash> targetTrigrams;
//    for (size_t i = 0; i < stdTarget.size() - 2; ++i) {
//        targetTrigrams.insert({&stdTarget.c_str()[i]});
//    }
//    std::list<size_t> neccesaryFiles;
//    if (trigramsInFiles.count(*targetTrigrams.begin()) == 0) {
//        return {};
//    }
//    for (size_t fileId : trigramsInFiles.at(*targetTrigrams.begin())) {
//        neccesaryFiles.push_back(fileId);
//    }
//    for (Trigram const &trigram : targetTrigrams) {
//        if (trigramsInFiles.count(trigram) > 0) {
//            mergeVectorToList(neccesaryFiles, trigramsInFiles.at(trigram));
//        }
//    }
//    std::vector<SubstringOccurrence> resultFiles;
//    for (size_t fileId : neccesaryFiles) {
//        resultFiles.push_back(
//            {documents[fileId].filename,
//             findExactOccurrences(documents[fileId], stdTarget)});
//    }
//    return resultFiles;
//}
