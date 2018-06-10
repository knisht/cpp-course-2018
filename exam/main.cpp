#include "list.hpp"
#include <iostream>
#include <list>

using namespace std;
using namespace exam;

template <typename T>
void check_equality(List<T> const &target, list<T> const &true_list, string descr)
{
    if (target.size() != true_list.size()) {
        cout << "=====>Fail at \"" << descr << "\".<=====\n";
        return;
    }
    auto it1 = target.begin();
    auto it2 = true_list.begin();
    for (; it1 != target.end() || it2 != true_list.end(); ++it1, ++it2) {
        if (*it1 != *it2) {
            //cout << "(" << *it1 << " vs " << *it2 << ")";
            cout << "=====>Fail at \"" << descr << "\".<=====\n";
            return;
        } else {
            //cout << "(" << *it1 << " vs " << *it2 << ")";
        }
    }
    auto it3 = target.rbegin();
    auto it4 = true_list.rbegin();
    for (; it3 != target.rend() || it4 != true_list.rend(); it3++, it4++) {
        if (*it3 != *it4) {
            cout << "=====>Fail at \"" << descr << "\".<=====\n";
            return;
        }
    }
    cout << "Test \"" << descr << "\" succesfully passed!\n";
}

template <typename T>
void check_equality(T x, T y, string descr)
{
    if (x != y) {
        cout << "=====>Fail at \"" << descr << "\".<=====\n";
    } else {
        cout << "Test \"" << descr << "\" succesfully passed!\n";
    }
}

int main()
{
    List<int> ilist;
    list<int> trueIlist;
    List<string> slist;
    list<string> trueSlist;
    for (int i = 0; i < 5; i += 2) {
        ilist.push_back(i);
        ilist.push_front(i + 1);
        trueIlist.push_back(i);
        trueIlist.push_front(i + 1);
        slist.push_back("a");
        slist.push_front("b");
        trueSlist.push_back("a");
        trueSlist.push_front("b");
    }
    check_equality(ilist, trueIlist, "int push_back & push_front");
    check_equality(slist, trueSlist, "string push_back & push_front");

    List<int> cpy1 = ilist;
    list<int> true_cpy1 = trueIlist;
    check_equality(cpy1, true_cpy1, "copy constructor");
    List<string> cpy2{};
    cpy2.push_back("1");
    cpy2 = slist;
    check_equality(cpy2, trueSlist, "copy assignment");

    for (int i = 0; i < 5; ++i) {
        ilist.pop_back();
        slist.pop_front();
        trueIlist.pop_back();
        trueSlist.pop_front();
    }
    check_equality(ilist, trueIlist, "int pop_back");
    check_equality(slist, trueSlist, "string pop_front");

    List<int> nli;
    List<string> nls;
    list<int> tli;
    list<string> tls;
    for (int i = 0; i < 10; ++i) {
        nli.push_front(i);
        nls.push_back(string({(char)('a' + i)}));
        tli.push_front(i);
        tls.push_back(string({(char)('a' + i)}));
    }
    nli.insert(nli.end(), 2);
    auto it = nli.begin();
    advance(it, 3);
    it = nli.insert(it, 4);
    std::advance(it, 5);
    it = nli.insert(it, 100);
    tli.insert(tli.end(), 2);
    auto it2 = tli.begin();
    advance(it2, 3);
    it2 = tli.insert(it2, 4);
    std::advance(it2, 5);
    it2 = tli.insert(it2, 100);
    check_equality(*it2, *it, "int insert iter equality");
    check_equality(nli, tli, "int inserts");

    auto it3 = nls.begin();
    advance(it3, 3);
    it3 = nls.insert(it3, "4");
    std::advance(it3, 5);
    it3 = nls.insert(it3, "100");
    auto it4 = tls.begin();
    advance(it4, 3);
    it4 = tls.insert(it4, "4");
    std::advance(it4, 5);
    it4 = tls.insert(it4, "100");
    check_equality(*it3, *it4, "int insert iter equality");
    check_equality(nli, tli, "string inserts");

    nli.erase(--nli.end());
    auto it5 = nli.begin();
    it5 = nli.erase(it5);
    std::advance(it5, 2);
    it5 = nli.erase(it5);

    tli.erase(--tli.end());
    auto it6 = tli.begin();
    it6 = tli.erase(it6);
    std::advance(it6, 2);
    it6 = tli.erase(it6);
    check_equality(*it5, *it6, "int erase iter equality");
    check_equality(nli, tli, "int erases");

    auto it7 = nls.begin();
    it7 = nls.erase(it7);
    std::advance(it7, 2);
    it7 = nls.erase(it7);

    auto it8 = tls.begin();
    it8 = tls.erase(it8);
    std::advance(it8, 2);
    it8 = tls.erase(it8);
    check_equality(*it7, *it8, "string erase iter equality");
    check_equality(nls, tls, "string erases");

    nli.clear();
    tli.clear();
    check_equality(nli, tli, "clear");

    for (int i = 0; i < 10; ++i) {
        nli.push_back(i * 2);
        tli.push_back(i * 2);
    }
    nli.erase(nli.begin(), nli.end());
    tli.erase(tli.begin(), tli.end());
    check_equality(nli, tli, "range erase");

    for (int i = 0; i < 10; ++i) {
        nli.push_back(i * 3);
        tli.push_back(i * 3);
    }

    List<int> nli2;
    std::list<int> tli2;
    nli2.splice(nli2.end(), nli, ++nli.begin(), --nli.end());
    tli2.splice(tli2.end(), tli, ++tli.begin(), --tli.end());

    check_equality(nli2, tli2, "splice");

    nli.clear();
    tli.clear();
    for (int i = 0; i < 100; ++i) {
        nli.push_back(i * 4);
        tli.push_back(i * 43);
    }

    List<int> nli3;
    std::list<int> tli3;
    nli3.splice(nli3.end(), nli, ++nli.begin(), ++nli.begin());
    tli3.splice(tli3.end(), tli, ++tli.begin(), ++tli.begin());

    check_equality(nli3, tli3, "splice2");

    nli.clear();
    tli.clear();
    for (int i = 0; i < 100; ++i) {
        nli.push_back(i);
        tli.push_back(i);
    }
    auto itt = nli.begin();
    std::advance(itt, 4);
    auto itt2 = tli.begin();
    std::advance(itt2, 4);
    nli.splice(--nli.end(), nli, nli.begin(), itt);
    tli.splice(--tli.end(), tli, tli.begin(), itt2);
    check_equality(nli, tli, "self-splice");
    nli.splice(nli.end(), nli, nli.begin(), --nli.end());
    tli.splice(tli.end(), tli, tli.begin(), --tli.end());
    check_equality(nli, tli, "self-splice2");

    nli.clear();
    tli.clear();
    for (int i = 0; i < 100; ++i) {
        nli.push_back(i);
        tli.push_back(i);
    }

    nli.splice(nli.end(), nli, nli.begin(), nli.end());
    tli.splice(tli.end(), tli, tli.begin(), tli.end());
    check_equality(nli, tli, "self-splice3");

    return 0;
}
