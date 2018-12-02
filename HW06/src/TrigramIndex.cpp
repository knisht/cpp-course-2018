#include "include/TrigramIndex.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <algorithm>
#include <functional>
#include <iostream>
#include <unordered_set>
#include <utility>

size_t TrigramIndex::Trigram::encode(const char *target) const
{
    return static_cast<size_t>(target[0] << 16) +
           (static_cast<size_t>(target[1]) << 8) +
           (static_cast<size_t>(target[2]));
}

std::string TrigramIndex::Trigram::toString() const
{
    return {static_cast<char>(trigram_code >> 16),
            static_cast<char>(trigram_code >> 8),
            static_cast<char>(trigram_code)};
}

bool TrigramIndex::Trigram::substr(std::string const &target) const
{
    if (target.size() >= 3) {
        return false;
    }
    if (target.size() == 2) {
        size_t target_code = static_cast<size_t>((target[0] << 8) + target[1]);
        if (target_code == (trigram_code & ((1 << 16) - 1))) {
            return true;
        } else {
            return false;
        }
    } else {
        return static_cast<size_t>(target[0]) ==
               (trigram_code & ((1 << 8) - 1));
    }
}

const qint32 BUF_SIZE = 1 << 20;

std::vector<TrigramIndex::Document> getFileEntries(QString const &root);

void unwrapTrigrams(TrigramIndex::Document &document);

TrigramIndex::TrigramIndex(QString const &root)
{
    auto start = std::chrono::steady_clock::now();
    documents = getFileEntries(root);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    std::cout << "Files collecting finished in " << duration.count() << "ms"
              << std::endl;
    for (size_t i = 0; i < documents.size(); ++i) {
        start = std::chrono::steady_clock::now();
        unwrapTrigrams(documents[i]);
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        std::cout << "Unwrapping of " << documents[i].filename.toStdString()
                  << " finished in " << duration.count() << "ms" << std::endl;
        for (auto &pair : documents[i].trigramOccurrences) {
            trigramsInFiles[pair.first].push_back(i);
        }

        duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        std::cout << "Collecting finished in " << duration.count() << "ms"
                  << std::endl;
    }
}

std::vector<TrigramIndex::Document> getFileEntries(QString const &root)
{
    QDirIterator dirIterator(root, QDir::NoFilter,
                             QDirIterator::Subdirectories);
    std::vector<TrigramIndex::Document> documents;
    while (dirIterator.hasNext()) {
        dirIterator.next();
        if (dirIterator.fileInfo().isDir()) {
            continue;
        }
        documents.push_back(
            TrigramIndex::Document(QFile(dirIterator.filePath()).fileName()));
    }
    return documents;
}

void unwrapTrigrams(TrigramIndex::Document &document)
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

    std::vector<std::pair<size_t, size_t>> trigramCodes(
        qMin(fileSize, block_size) - 2);
    while (fileSize > 0) {
        fileInstance.read(bytes.data(), block_size);
        if (passed > 0) {
            last[2] = bytes[0];
            document.trigramOccurrences[{last}].push_back(
                static_cast<size_t>(passed * block_size - 2));
            char trigramBuf[3] = {last[1], last[2], bytes[1]};
            document.trigramOccurrences[{trigramBuf}].push_back(
                static_cast<size_t>(passed * block_size - 1));
        }

        auto start = std::chrono::steady_clock::now();
        //#pragma omp parallel for
        for (size_t i = 0; i < trigramCodes.size(); ++i) {
            trigramCodes[i] = {
                static_cast<size_t>(bytes.data()[i] << 16) +
                    static_cast<size_t>(bytes.data()[i + 1] << 8) +
                    static_cast<size_t>(bytes.data()[i + 2]),
                passed * block_size + i};
        }
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        std::cout << "Parallelling finished in " << duration.count() << "ms"
                  << std::endl;
        for (size_t i = 0; i < trigramCodes.size(); ++i) {
            // TODO: smth with this
            document.trigramOccurrences[trigramCodes[i].first].push_back(
                trigramCodes[i].second);
        }
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        std::cout << "Files reading finished in " << duration.count() << "ms"
                  << std::endl;
        if (document.trigramOccurrences.size() > 300000) {
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

void mergeVectorToList(std::list<size_t> &destination,
                       std::vector<size_t> const &source)
{
    auto it = destination.begin();
    for (size_t currentIndex : source) {
        while (it != destination.end() && *it < currentIndex) {
            destination.erase(it++);
        }
        if (it == destination.end()) {
            return;
        }
        if (*it == currentIndex) {
            ++it;
        }
    }
}

std::vector<size_t> findExactOccurrences(TrigramIndex::Document const &doc,
                                         std::string const &target)
{
    QFile fileInstance(doc.filename);
    fileInstance.open(QFile::ReadOnly);
    size_t currentPosition = 0;
    TrigramIndex::Trigram start{target};
    std::string buf(target.size(), '\0');
    std::vector<size_t> result;
    if (doc.trigramOccurrences.count(start) != 0) {
        for (size_t occurrence : doc.trigramOccurrences.at(start)) {
            if (occurrence < currentPosition) {
                size_t delta = currentPosition - occurrence;
                for (size_t i = 0; i < delta; ++i) {
                    buf[i] = buf[i + target.size() - delta];
                }
                fileInstance.read(&buf[0] + delta,
                                  static_cast<int>(target.size() - delta));
            } else {
                fileInstance.skip(
                    static_cast<int>(occurrence - currentPosition));
                fileInstance.read(&buf[0], static_cast<int>(target.size()));
            }
            if (strncmp(&buf[0], &target[0], target.size()) == 0) {
                result.push_back(occurrence);
            }
            currentPosition = occurrence + target.size();
        }
    }
    return result;
}

std::vector<TrigramIndex::SubstringOccurrence>
TrigramIndex::smallStringProcess(std::string const &target) const
{
    std::unordered_set<size_t> fileIds;
    for (auto &pair : trigramsInFiles) {
        if (pair.first.substr(target)) {
            fileIds.insert(pair.second.begin(), pair.second.end());
        }
    }
    std::vector<TrigramIndex::SubstringOccurrence> result;
    for (size_t fileId : fileIds) {
        std::vector<size_t> occurrences;
        for (auto &pair : documents[fileId].trigramOccurrences) {
            if (pair.first.substr(target)) {
                for (size_t j : pair.second) {
                    occurrences.push_back(j);
                }
            }
        }
        // TODO: end of file trigrams
        // TODO: move occurrs
        result.push_back({documents[fileId].filename, occurrences});
    }
    for (auto &it : result) {
        std::sort(it.occurrences.begin(), it.occurrences.end());
        it.occurrences.erase(
            std::unique(it.occurrences.begin(), it.occurrences.end()),
            it.occurrences.end());
    }
    return result;
}

std::vector<TrigramIndex::SubstringOccurrence>
TrigramIndex::findSubstring(QString const &target) const
{
    std::string stdTarget = target.toStdString();
    if (stdTarget.size() <= 2) {
        return smallStringProcess(stdTarget);
    }
    std::unordered_set<TrigramIndex::Trigram, TrigramHash> targetTrigrams;
    for (size_t i = 0; i < stdTarget.size() - 2; ++i) {
        targetTrigrams.insert({&stdTarget.c_str()[i]});
    }
    std::list<size_t> neccesaryFiles;
    if (trigramsInFiles.count(*targetTrigrams.begin()) == 0) {
        return {};
    }
    for (size_t fileId : trigramsInFiles.at(*targetTrigrams.begin())) {
        neccesaryFiles.push_back(fileId);
    }
    for (Trigram const &trigram : targetTrigrams) {
        if (trigramsInFiles.count(trigram) > 0) {
            mergeVectorToList(neccesaryFiles, trigramsInFiles.at(trigram));
        }
    }
    std::vector<TrigramIndex::SubstringOccurrence> resultFiles;
    for (size_t fileId : neccesaryFiles) {
        resultFiles.push_back(
            {documents[fileId].filename,
             findExactOccurrences(documents[fileId], stdTarget)});
    }
    return resultFiles;
}
