#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int main() {
    string line;
    getline(cin, line);
    stringstream ss(line);
    int num;
    bool first = true;
    
    while (ss >> num) {
        if (!first) cout << " ";
        first = false;
        cout << num * 7;
    }
    cout << endl;
    return 0;
}