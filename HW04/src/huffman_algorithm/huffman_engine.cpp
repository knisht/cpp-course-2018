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
    for (size_t c = 0; c < 256; ++c) {
        frequencies[c] = 0;
    }
}

void huffman_engine::add_all(const std::string &str)
{
    for (size_t i = 0; i < str.length(); ++i) {
        ++frequencies[static_cast<word_type>(str[i])];
    }
}

huffman_engine::Huffman_tree huffman_engine::generate_code()
{
    std::vector<Node> heap;
    for (size_t i = 0; i < 256; ++i) {
        if (frequencies[i] > 0)
            heap.push_back({SIZE_MAX, SIZE_MAX, frequencies[i], static_cast<word_type>(i)});
    }
    Huffman_tree result;
    if (heap.size() == 0)
        return result;
    std::sort(heap.begin(), heap.end(), [](Node a, Node b) { return a.weight < b.weight; });
    std::vector<Node> helpheap;
    size_t index = 0;
    size_t itf = 0;
    size_t its = SIZE_MAX;
    for (size_t i = 0; i < heap.size() - 1; ++i) {
        Node first;
        Node second;
        if (heap.empty() || itf == heap.size()) {
            first = helpheap[its];
            ++its;
        } else if ((helpheap.empty() || its == helpheap.size()) || (heap[itf].weight < helpheap[its].weight)) {
            first = heap[itf];
            ++itf;
        } else {
            first = helpheap[its];
            ++its;
        }
        result[index] = first;
        ++index;
        if (heap.empty() || itf == heap.size()) {
            second = helpheap[its];
            ++its;
        } else if ((helpheap.empty() || its == helpheap.size()) || (heap[itf].weight < helpheap[its].weight)) {
            second = heap[itf];
            ++itf;
        } else {
            second = helpheap[its];
            ++its;
        }
        result[index] = second;
        ++index;
        helpheap.push_back({index - 2u, index - 1u, result[index - 1].weight + result[index - 2].weight, 0});
        if (its == SIZE_MAX)
            its = 0;
    }
    if (heap.size() == 1)
        helpheap.push_back(heap[0]);
    result[index] = helpheap.back();
    result.root = result[index];
    result.empty = false;
    return result;
}

void huffman_engine::set_tree(huffman_engine::Huffman_tree const &target_tree)
{
    tree = target_tree;
    if (target_tree.empty)
        return;
    std::queue<std::pair<Node, bitstring>> queue;
    queue.push({target_tree.root, {}});
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

void huffman_engine::encode(std::string const &source, bitstring &past)
{
    past.reserve(source.length() * 8);
    for (size_t i = 0; i < source.length(); ++i) {
        past.append(code[static_cast<word_type>(source[i])]);
    }
}

std::string huffman_engine::decode(bitstring &source, size_t length)
{
    Node current_vertex = tree.root;
    char result[1 << 20];

    size_t lastindex = 0;
    size_t limit = ((length == 0) ? source.length() : length);
    size_t resindex = 0;
    for (size_t i = 0; i < limit; ++i) {
        if (current_vertex.left_child != SIZE_MAX) {
            if (source[i])
                current_vertex = tree[current_vertex.right_child];
            else
                current_vertex = tree[current_vertex.left_child];
        }
        if (current_vertex.left_child == SIZE_MAX) {
            lastindex = i;
            result[resindex] = static_cast<char>(current_vertex.word);
            ++resindex;
            current_vertex = tree.root;
        }
    }
    bitstring rest;
    for (size_t i = lastindex + 1; i < source.length(); ++i) {
        rest.append(source[i]);
    }
    source = rest;
    return std::string(result, resindex);
}

void huffman_engine::get_dictionary_representation(bitstring &order, bitstring &leaves)
{
    order = bitstring{};
    leaves = bitstring{};
    if (tree.empty)
        return;
    tree_bypass(order, leaves, tree.root, true);
}

huffman_engine::Huffman_tree huffman_engine::decode_dictionary(bitstring const &order, bitstring const &leaves)
{
    if (leaves.length() == 0)
        return Huffman_tree{};
    bool visited[512];
    bool moveright[512];
    size_t stack[512];
    size_t encounter = 0;
    memset(visited, 0, sizeof(bool) * 512);
    memset(moveright, 0, sizeof(bool) * 512);
    Huffman_tree result;
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
                result[resultsize] = {SIZE_MAX, SIZE_MAX, 0, static_cast<word_type>(leaves.get_ordered_char(wordcounter))};
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
    if (resultsize == 0) {
        result[resultsize] = {SIZE_MAX, SIZE_MAX, 0, static_cast<word_type>(leaves.get_ordered_char(0))};
    }
    result.root = result[0];
    result.empty = false;
    return result;
}

size_t huffman_engine::getlen()
{
    size_t accum = 0;
    for (size_t i = 0; i < 256; ++i) {
        accum += frequencies[i] * code[i].length();
    }
    return accum;
}

void huffman_engine::flush()
{
    for (size_t i = 0; i < 256; ++i) {
        frequencies[i] = 0;
    }
}

void huffman_engine::tree_bypass(bitstring &result, bitstring &leaves, huffman_engine::Node const &vertex, bool is_root)
{
    if (vertex.left_child != SIZE_MAX) {
        result.append(false);
        tree_bypass(result, leaves, tree[vertex.left_child], false);
        result.append(false);
        tree_bypass(result, leaves, tree[vertex.right_child], false);
    } else {
        leaves.append(vertex.word);
    }
    if (!is_root)
        result.append(true);
}
