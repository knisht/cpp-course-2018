#include "include/TrigramIndex.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <vector>

const qint32 BUF_SIZE = 1 << 20;

std::vector<TrigramIndex::Document> getFileEntries(QString const &root);

void unwrapTrigrams(TrigramIndex::Document &document);

TrigramIndex::TrigramIndex(QString const &root)
{
    documents = getFileEntries(root);
    for (size_t i = 0; i < documents.size(); ++i) {
        unwrapTrigrams(documents[i]);
        for (auto &trigram : documents[i].trigramOccurrences) {
            if (trigramsInFiles.count(trigram.first) == 0) {
                trigramsInFiles[trigram.first] = std::vector<size_t>{};
            }
            trigramsInFiles[trigram.first].push_back(i);
        }
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
    size_t unique_trigrams = 0;
    while (fileSize > 0) {
        fileInstance.read(bytes.data(), block_size);
        if (passed > 0) {
            last[2] = bytes[0];
            TrigramIndex::Trigram trigram{last};
            if (document.trigramOccurrences.count(trigram) == 0) {
                ++unique_trigrams;
                document.trigramOccurrences[trigram] = std::vector<size_t>{};
            }
            document.trigramOccurrences[trigram].push_back(
                static_cast<size_t>(passed * block_size - 2));
            char trigramBuf[3] = {last[1], last[2], bytes[1]};
            trigram = TrigramIndex::Trigram(trigramBuf);
            if (document.trigramOccurrences.count(trigram) == 0) {
                ++unique_trigrams;
                document.trigramOccurrences[trigram] = std::vector<size_t>{};
            }
            document.trigramOccurrences[trigram].push_back(
                static_cast<size_t>(passed * block_size - 1));
        }
        for (int i = 0; i < qMin(fileSize, block_size) - 2; ++i) {
            // FIXME: Copypast
            // FIXME: Slow
            TrigramIndex::Trigram trigram{bytes.data() + i};
            if (document.trigramOccurrences.count(trigram) == 0) {
                ++unique_trigrams;
                document.trigramOccurrences[trigram] = std::vector<size_t>{};
            }
            document.trigramOccurrences[trigram].push_back(
                static_cast<size_t>(passed * block_size + i));
        }
        if (unique_trigrams > 300000) {
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
        if (it == destination.end()) {
            break;
        }
        while (*it < currentIndex) {
            destination.erase(it);
            ++it;
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
                size_t delta = target.size() - currentPosition + occurrence;
                for (size_t i = 0; i < delta; ++i) {
                    buf[i] = buf[i + delta];
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
TrigramIndex::smallStringProcess(std::string target) const
{
    std::set<size_t> fileIds;
    for (auto &pair : trigramsInFiles) {
        for (size_t i = 0; i < 4 - target.size(); ++i) {
            if (strncmp(target.data(), pair.first.data() + i, target.size())) {
                fileIds.insert(pair.second.begin(), pair.second.end());
                break;
            }
        }
    }
    std::vector<TrigramIndex::SubstringOccurrence> result;
    for (size_t fileId : fileIds) {
        std::vector<size_t> occurrences;
        for (auto &pair : documents[fileId].trigramOccurrences) {
            for (size_t i = 0; i < 4 - target.size(); ++i) {
                // FIXME: Copypast
                if (strncmp(target.data(), pair.first.data() + i,
                            target.size())) {
                    occurrences.insert(occurrences.end(), pair.second.begin(),
                                       pair.second.end());
                    break;
                }
            }
        }
        result.push_back({documents[fileId].filename, occurrences});
    }
    return {};
}

std::vector<TrigramIndex::SubstringOccurrence>
TrigramIndex::findSubstring(QString const &target) const
{
    std::string stdTarget = target.toStdString();
    if (stdTarget.size() <= 2) {
        return smallStringProcess(stdTarget);
    }
    std::set<TrigramIndex::Trigram> targetTrigrams;
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
