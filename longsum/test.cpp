#include <iostream>
#include <vector>

using namespace std;

int main(void) {
    vector<int> first(3);
    vector<int> auxiliary(first.size());
    for (auto i: auxiliary)
        cout << i << " ";
}