#include "include/core/checker.hpp"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <utility>
#include <vector>
namespace core
{
const size_t CHAR_BUF_SIZE = 65536;

namespace fs = std::filesystem;

const size_t max_depth = 10;

std::vector<std::string> get_filenames(std::string const &path,
                                       size_t depth = 0)
{
    if (depth > max_depth) {
        return {};
    }
    std::vector<std::string> filenames;
    for (auto &p : fs::directory_iterator(path)) {
        try {
            if (fs::is_directory(p)) {
                std::vector<std::string> inner =
                    get_filenames(p.path(), depth + 1);
                filenames.insert(filenames.end(), inner.begin(), inner.end());

            } else if (fs::is_symlink(p)) {
                if (fs::exists(fs::canonical(p))) {
                    filenames.push_back(p.path());
                }
            } else {
                filenames.push_back(p.path());
            }
        } catch (fs::filesystem_error e) {
            std::cerr << e.what() << std::endl;
        }
    }
    return filenames;
}

size_t get_hash(std::string const &filepath)
{
    std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
    size_t available = static_cast<size_t>(ifs.tellg());
    char buf[CHAR_BUF_SIZE + 1];
    memset(buf, 0, CHAR_BUF_SIZE);
    buf[CHAR_BUF_SIZE] = '\0';
    size_t result = 0;
    ifs = std::ifstream(filepath, std::ios::binary);
    while (available > 0) {
        size_t left = std::min(CHAR_BUF_SIZE, available);
        available -= left;
        ifs.read(buf, CHAR_BUF_SIZE);
        result += std::hash<std::string>{}(buf);
    }
    return result;
}

bool equal(std::pair<std::string, size_t> const &a,
           std::pair<std::string, size_t> const &b)
{
    if (a.second != b.second) {
        return false;
    }
    if (fs::file_size(a.first) != fs::file_size(b.first)) {
        return false;
    }
    size_t file_length = fs::file_size(a.first);
    std::ifstream first_ifs(a.first, std::ios::binary),
        second_ifs(b.first, std::ios::binary);
    char first_buf[CHAR_BUF_SIZE + 1], second_buf[CHAR_BUF_SIZE + 1];
    first_buf[CHAR_BUF_SIZE] = '\0';
    second_buf[CHAR_BUF_SIZE] = '\0';
    memset(first_buf, 0, CHAR_BUF_SIZE);
    memset(second_buf, 0, CHAR_BUF_SIZE);
    while (file_length > 0) {
        size_t left = std::min(CHAR_BUF_SIZE, file_length);
        file_length -= left;
        first_ifs.read(first_buf, CHAR_BUF_SIZE);
        second_ifs.read(second_buf, CHAR_BUF_SIZE);
        if (strcmp(first_buf, second_buf) != 0) {
            return false;
        }
    }
    return true;
}

std::vector<std::vector<std::string>>
group_everything(std::string const &path, std::string const &root = "")
{
    // Maybe std::vector will be better cause of small number of files
    std::list<std::pair<std::string, size_t>> filenames;
    std::string directory = root == "" ? path : root;
    for (std::string const &filename : get_filenames(directory)) {
        filenames.push_back(make_pair(filename, get_hash(filename)));
    }
    std::vector<std::vector<std::string>> groups;
    if (root == "") {
        while (!filenames.empty()) {
            std::pair<std::string, size_t> file = *filenames.begin();
            filenames.erase(filenames.begin());
            groups.push_back(std::vector<std::string>{file.first});
            std::vector<std::list<std::pair<std::string, size_t>>::iterator>
                iters;
            for (auto file_iter = filenames.begin();
                 file_iter != filenames.end(); ++file_iter) {
                if (equal(*file_iter, file)) {
                    groups.back().push_back(file_iter->first);
                    iters.push_back(file_iter);
                }
            }
            for (auto iter : iters) {
                filenames.erase(iter);
            }
        }
    } else {
        auto mainfile = std::find(filenames.begin(), filenames.end(),
                                  make_pair(path, get_hash(path)));
        groups.push_back({});
        for (auto &&it : filenames) {
            if (equal(it, *mainfile)) {
                groups.back().push_back(it.first);
            }
        }
    }
    return groups;
}

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
