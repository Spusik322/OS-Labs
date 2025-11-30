#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int main() {
    string line;
    getline(cin, line);
    stringstream ss(line);
    long long sum = 0;
    int num;
    
    while (ss >> num) {
        sum += num;
    }
    cout << sum << endl;
    return 0;
}