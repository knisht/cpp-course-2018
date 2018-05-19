#include "../../include/huffman_engine.h"
#include "../../include/bitstring.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <utility>

using word_type = uint8_t;

huffman_engine::huffman_engine() : code()
{
    for (size_t c = 0; c <= 255; ++c) {
        frequencies[c] = 0;
        if (c == 255)
            break;
    }
}

void huffman_engine::add(char word)
{
    ++frequencies[reinterpret_cast<word_type &>(word)];
}

void huffman_engine::add_all(const std::string &str)
{
    for (size_t i = 0; i < str.length(); ++i) {
        add(str[i]);
    }
}

std::vector<huffman_engine::Node> huffman_engine::generate_code()
{
    std::vector<Node> heap;
    for (size_t i = 0; i <= 255; ++i) {
        if (frequencies[i] > 0)
            heap.push_back({SIZE_MAX, SIZE_MAX, frequencies[i], static_cast<word_type>(i)});
        if (i == 255)
            break;
    }
    std::vector<Node> storage;
    if (heap.size() == 0)
        return storage;
    std::sort(heap.begin(), heap.end(), [](Node a, Node b) { return a.weight < b.weight; });
    std::vector<Node> helpheap;
    storage.resize(512);
    size_t index = 0;
    size_t itf = 0;
    size_t its = SIZE_MAX;
    for (size_t i = 0; i < heap.size() - 1; ++i) {
        Node first;
        Node second;
        if (heap.empty() || itf == heap.size()) {
            first = helpheap[its];
            ++its;
        } else if (helpheap.empty() || its == helpheap.size()) {
            first = heap[itf];
            ++itf;
        } else if (heap[itf].weight < helpheap[its].weight) {
            first = heap[itf];
            ++itf;
        } else {
            first = helpheap[its];
            ++its;
        }
        storage[index] = first;
        ++index;
        if (heap.empty() || itf == heap.size()) {
            second = helpheap[its];
            ++its;
        } else if (helpheap.empty() || its == helpheap.size()) {
            second = heap[itf];
            ++itf;
        } else if (heap[itf].weight < helpheap[its].weight) {
            second = heap[itf];
            ++itf;
        } else {
            second = helpheap[its];
            ++its;
        }
        storage[index] = second;
        ++index;
        helpheap.push_back({index - 2u, index - 1u, storage[index - 1].weight + storage[index - 2].weight, 0});
        if (its == SIZE_MAX)
            its = 0;
    }
    if (heap.size() == 1)
        helpheap.push_back(heap[0]);
    storage[index] = helpheap.back();
    tree_root = storage[index];
    ++index;
    storage.resize(index);
    return storage;
}

void huffman_engine::encode(std::string const &source, bitstring &past)
{
    for (size_t i = 0; i < source.length(); ++i) {
        word_type symbol = static_cast<word_type>(source[i]);
        past.append(code[symbol]);
    }
}

void huffman_engine::set_tree(std::vector<Node> target_tree)
{
    tree = target_tree;
    if (target_tree.size() == 0)
        return;
    std::queue<std::pair<Node, bitstring>> queue;
    queue.push({tree_root, {}});
    while (!queue.empty()) {
        std::pair<Node, bitstring> vertex = queue.front();
        queue.pop();
        if (!(vertex.first.left_child != SIZE_MAX)) {
            if (vertex.second.length() == 0)
                vertex.second.append(false);
            code[vertex.first.word] = vertex.second;
        } else {
            bitstring leftstring = vertex.second;
            leftstring.append(false);
            bitstring rightstring = vertex.second;
            rightstring.append(true);
            queue.push({tree[vertex.first.left_child], leftstring});
            queue.push({tree[vertex.first.right_child], rightstring});
        }
    }
}

std::string huffman_engine::decode(bitstring &source, size_t length)
{
    Node current_vertex = tree_root;
    std::string result;
    size_t lastindex = 0;
    for (size_t i = 0; i < ((length == 0) ? source.length() : length); ++i) {
        if (current_vertex.left_child != SIZE_MAX) {
            if (source[i])
                current_vertex = tree[current_vertex.right_child];
            else
                current_vertex = tree[current_vertex.left_child];
        }
        if (current_vertex.left_child == SIZE_MAX) {
            lastindex = i;
            result += (reinterpret_cast<char &>(current_vertex.word));
            current_vertex = tree_root;
        }
    }
    bitstring rest;
    for (size_t i = lastindex + 1; i < source.length(); ++i) {
        rest.append(source[i]);
    }
    source = rest;
    return result;
}

void huffman_engine::tree_bypass(bitstring &result, bitstring &leaves, huffman_engine::Node const &vertex)
{
    if (vertex.left_child != SIZE_MAX) {
        result.append(false);
        tree_bypass(result, leaves, tree[vertex.left_child]);
        result.append(false);
        tree_bypass(result, leaves, tree[vertex.right_child]);
    } else {
        leaves.append(vertex.word);
    }
    if (tree.back().weight != vertex.weight)
        result.append(true);
}

void huffman_engine::get_dictionary_representation(bitstring &order, bitstring &leaves)
{
    order = bitstring{};
    leaves = bitstring{};
    if (tree.empty())
        return;
    tree_bypass(order, leaves, tree_root);
}

std::vector<huffman_engine::Node> huffman_engine::decode_dictionary(bitstring const &order, bitstring const &leaves)
{
    bool visited[512];
    bool moveright[512];
    size_t stack[512];
    size_t encounter = 0;
    memset(visited, 0, sizeof(bool) * 512);
    memset(moveright, 0, sizeof(bool) * 512);
    Node result[512];
    size_t resultsize = 0;
    size_t wordcounter = 0;
    for (size_t i = 0; i < order.length(); ++i) {
        if (!order[i]) {
            if (!visited[encounter]) {
                stack[encounter] = resultsize;
                moveright[encounter] = false;
                result[resultsize] = {0, 0, 0, 0};
                ++resultsize;
                visited[encounter] = true;
                ++encounter;
            } else {
                moveright[encounter] = true;
                ++encounter;
            }
        } else {
            if (!visited[encounter]) {
                result[resultsize] = {SIZE_MAX, SIZE_MAX, 0, static_cast<word_type>(leaves.get_char(wordcounter))};
                stack[encounter] = resultsize;
                ++wordcounter;
                ++resultsize;
            }
            if (moveright[encounter - 1])
                result[stack[encounter - 1]].right_child = stack[encounter];
            else
                result[stack[encounter - 1]].left_child = stack[encounter];
            visited[encounter] = false;
            --encounter;
        }
    }
    std::vector<Node> res;
    for (size_t i = 0; i < resultsize; ++i) {
        res.push_back(result[i]);
    }
    if (resultsize == 0) {
        res.push_back({SIZE_MAX, SIZE_MAX, 0, static_cast<word_type>(leaves.get_char(0))});
    }
    tree_root = res[0];
    return res;
}

unsigned long long huffman_engine::getlen()
{
    unsigned long long accum = 0;
    for (size_t i = 0; i <= 255; ++i) {
        accum += frequencies[i] * code[i].length();
        if (i == 255)
            break;
    }
    return accum;
}

void huffman_engine::flush()
{
    for (size_t i = 0; i <= 255; ++i) {
        frequencies[i] = 0;
        if (i == 255)
            break;
    }
}
