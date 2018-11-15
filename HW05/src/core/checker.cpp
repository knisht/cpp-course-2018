#include "include/core/checker.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <queue>
#include <unordered_set>
#include <utility>
#include <vector>

namespace core
{
const size_t CHAR_BUF_SIZE = 1 << 20;

namespace
{

namespace fs = std::filesystem;
// bfs
std::vector<std::pair<std::string, size_t>>
get_filenames(std::string const &path)
{
    std::vector<std::pair<std::string, size_t>> filenames;
    std::vector<std::string> queue;
    std::unordered_set<std::string> processed_filenames;
    queue.push_back(path);
    size_t queue_front = 0;
    while (queue_front != queue.size()) {
        try {
            for (auto &&p : fs::directory_iterator(queue[queue_front])) {
                try {
                    std::string canonical_path = fs::canonical(p);
                    if (processed_filenames.count(canonical_path) != 0) {
                        continue;
                    }
                    if (fs::is_directory(canonical_path)) {
                        queue.push_back(canonical_path);
                    } else {
                        filenames.push_back(
                            {canonical_path, fs::file_size(canonical_path)});
                    }
                    processed_filenames.insert(canonical_path);
                } catch (fs::filesystem_error &e) {
                    std::cerr << "[ERROR] " << e.what() << std::endl;
                }
            }
        } catch (fs::filesystem_error &e) {
            std::cerr << "[ERROR] " << e.what() << std::endl;
        }
        ++queue_front;
    }
    return filenames;
}

size_t hashcheck = 0;

size_t get_hash(std::string const &filepath, size_t filesize)
{
    size_t result = 0;
    std::ifstream ifs = std::ifstream(filepath, std::ios::binary);
    if (filesize >= CHAR_BUF_SIZE) {
        std::string buf;
        buf.resize(CHAR_BUF_SIZE);
        while (filesize >= CHAR_BUF_SIZE) {
            filesize -= CHAR_BUF_SIZE;
            ifs.read(&buf[0], static_cast<std::streamsize>(CHAR_BUF_SIZE));
            result += std::hash<std::string>{}(buf);
        }
    }
    if (filesize > 0) {
        std::string buf;
        buf.resize(filesize);
        ifs.read(&buf[0], static_cast<std::streamsize>(filesize));
        result += std::hash<std::string>{}(buf);
    }
    return result;
}

struct file_brief {
    std::string filename;
    size_t size;
    size_t hash;

    bool operator==(file_brief const &other) const
    {
        return other.hash == hash && other.size == size &&
               other.filename == filename;
    }
};

bool equal(file_brief const &a, file_brief const &b)
{
    if (a.hash != b.hash) {
        return false;
    }
    if (a.size != b.size) {
        return false;
    }
    std::ifstream first_ifs(a.filename, std::ios::binary),
        second_ifs(b.filename, std::ios::binary);
    char first_buf[CHAR_BUF_SIZE], second_buf[CHAR_BUF_SIZE];
    size_t file_length = a.size;
    while (file_length > 0) {
        size_t left = std::min(CHAR_BUF_SIZE, file_length);
        file_length -= left;
        first_ifs.read(first_buf, CHAR_BUF_SIZE);
        second_ifs.read(second_buf, CHAR_BUF_SIZE);
        if (strncmp(first_buf, second_buf, left) != 0) {
            ++hashcheck;
            return false;
        }
    }
    return true;
}

std::vector<std::vector<std::string>> static group_everything(
    std::string const &path, std::string const &root = "")
{
    hashcheck = 0;
    std::chrono::high_resolution_clock::time_point t1 =
        std::chrono::high_resolution_clock::now();
    // Maybe std::vector will be better cause of small number of files
    std::string directory = root == "" ? path : root;
    std::vector<std::pair<std::string, size_t>> filenames =
        get_filenames(directory);
    std::unordered_map<size_t, size_t> occurrences;
    for (auto &&file_and_size : filenames) {
        ++occurrences[file_and_size.second];
    };
    std::list<file_brief> filtered_filenames;

    std::chrono::high_resolution_clock::time_point tk =
        std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(tk - t1).count();
    std::cout << "[INFO] Basic parsing of " << directory << " finished in "
              << duration << "ms" << std::endl;

    for (auto &&file_and_size : filenames) {
        if (occurrences[file_and_size.second] >= 2) {
            filtered_filenames.push_back(
                {file_and_size.first, file_and_size.second,
                 get_hash(file_and_size.first, file_and_size.second)});
        }
    }
    // chrono stuff
    std::chrono::high_resolution_clock::time_point tm =
        std::chrono::high_resolution_clock::now();
    duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(tm - t1).count();
    std::cout << "[INFO] Hashing of " << directory << " finished in "
              << duration << "ms" << std::endl;
    // chrono stuff

    std::vector<std::vector<std::string>> groups;
    filtered_filenames.sort([](file_brief const &a, file_brief const &b) {
        return a.size == b.size ? a.hash < b.hash : a.size < b.size;
    });
    if (root == "") {
        while (!filtered_filenames.empty()) {
            file_brief file = *filtered_filenames.begin();
            filtered_filenames.erase(filtered_filenames.begin());
            groups.push_back(std::vector<std::string>{file.filename});
            std::vector<std::list<file_brief>::iterator> iters;
            for (auto file_iter = filtered_filenames.begin();
                 file_iter != filtered_filenames.end(); ++file_iter) {
                if (file_iter->hash != file.hash ||
                    file_iter->size != file.size) {
                    break;
                }
                if (equal(*file_iter, file)) {
                    groups.back().push_back(file_iter->filename);
                    iters.push_back(file_iter);
                }
            }
            if (groups.back().size() == 1) {
                groups.pop_back();
            }
            for (auto iter : iters) {
                filtered_filenames.erase(iter);
            }
        }
    } else {
        auto mainfile =
            std::find(filtered_filenames.begin(), filtered_filenames.end(),
                      file_brief{path, fs::file_size(path),
                                 get_hash(path, fs::file_size(path))});
        groups.push_back({});
        for (auto &&it : filtered_filenames) {
            if (equal(it, *mainfile)) {
                groups.back().push_back(it.filename);
            }
        }
    }
    std::chrono::high_resolution_clock::time_point t2 =
        std::chrono::high_resolution_clock::now();

    duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "[INFO] Full scanning of " << directory << " finished in "
              << duration << "ms with " << hashcheck << " hash collisions"
              << std::endl;
    return groups;
}
} // namespace

std::vector<std::string> group_for(std::string const &path,
                                   const std::string &root)
{
    return group_everything(path, root)[0];
}

std::vector<std::vector<std::string>> group_all(std::string const &path)
{
    return group_everything(path);
}

} // namespace core
