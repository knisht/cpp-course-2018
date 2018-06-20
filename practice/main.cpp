#include "simple_vector.h"
#include <iostream>

using namespace std;
template <typename T>
void print(simple_vector<T> &v)
{
    for (size_t i = 0; i < v.size(); ++i) {
        cout << v[i] << " ";
    }
    cout << endl;
}

int main()
{

    simple_vector<int> a;
    print(a);
    simple_vector<string> b;
    print(b);

    cout << " === Empty init passed === \n";

    string tmp = string();

    simple_vector<int> c(10);
    print(c);
    simple_vector<string> d(10);
    print(d);

    cout << " === Number init passed === \n";

    simple_vector<int> e(c);
    print(e);
    simple_vector<string> f(d);
    print(f);

    cout << " === Copy ctor passed ===\n";

    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    b.push_back("hi");
    b.push_back("hello");
    b.push_back("bye");
    print(a);
    print(b);

    cout << " === push_back passed === \n";

    simple_vector<int> g = a;
    print(b);
    simple_vector<string> h;
    h = b;
    print(h);

    cout << " === copy assignment passed === \n";

    cout << g.size() << endl;
    cout << h.size() << endl;
    cout << " === size() passed === \n";

    g.pop_back();
    g.pop_back();
    h.pop_back();
    h.pop_back();
    print(g);
    print(h);
    cout << " === pop_back  passed === \n";

    g.reserve(1000);
    g.shrink_to_fit();
    cout << (g.capacity() == 1) << endl;
    cout << " === shrink_to_fit  passed === \n";

    h.push_back("100g");
    h.clear();
    cout << (h.size() == 0) << endl;
    cout << " === size()  passed === \n";

    h.push_back("e1");
    h.push_back("e2");
    h.push_back("e3");
    h.insert(h.begin() + 2, "inserted");
    print(h);
    cout << " === insert passed === \n";

    h.erase(h.begin() + 2);
    print(h);
    cout << " === erase passed === \n";

    h.push_back("e4");
    h.push_back("e5");
    h.erase(h.begin() + 1, h.begin() + 3);
    print(h);
    cout << " === range erase passed === \n";

    h.push_back("e6");
    h.push_back("e7");
    h.push_back("e8");
    h.resize(3);
    print(h);
    cout << " === resize passed === \n";
    return 0;
}
