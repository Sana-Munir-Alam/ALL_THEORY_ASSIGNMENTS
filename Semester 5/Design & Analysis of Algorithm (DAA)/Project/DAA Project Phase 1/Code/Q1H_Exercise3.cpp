#include <iostream>
#include <vector>
using namespace std;

vector<int> account;
long long tests = 0;

bool equivalent(int i, int j) { tests++; return account[i] == account[j]; }

int candidate(int l, int r) {
    if (l == r) return l;
    int m = (l + r) / 2;
    int cands[2] = {candidate(l, m), candidate(m + 1, r)};
    for (int c : cands) {
        if (c == -1) continue;
        int count = 0;
        for (int k = l; k <= r; k++)
            if (k == c || equivalent(c, k)) count++;
        if (2 * count > r - l + 1) return c;
    }
    return -1;
}

int main() {
    int n;
    cout << "Enter number of cards n (>= 10): "; cin >> n;
    if (n < 10) { cout << "n must be >= 10\n"; return 1; }
    
    account.resize(n);
    cout << "Enter an account number for each card: ";
    
    for (int &x : account) cin >> x;
    int c = candidate(0, n - 1);
    
    if (c == -1) cout << "No set of more than n/2 equivalent cards.\n";
    else cout << "YES: more than n/2 cards are equivalent to card " << c + 1 << "\n";
    cout << "Equivalence-tester calls = " << tests << "\n";
    return 0;
}