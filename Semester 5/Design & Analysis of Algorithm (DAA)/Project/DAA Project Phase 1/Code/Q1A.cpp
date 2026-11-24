#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
using namespace std;

long long cmpCount = 0;

pair<int, int> minMax(const vector<int>& A, int l, int r) {
    if (l == r) return {A[l], A[l]};
    if (r - l == 1) {
        cmpCount++;
        return A[l] < A[r] ? make_pair(A[l], A[r]) : make_pair(A[r], A[l]);
    }
    int m = (l + r) / 2;
    auto L = minMax(A, l, m);
    auto R = minMax(A, m + 1, r);
    cmpCount += 2;
    return {min(L.first, R.first), max(L.second, R.second)};
}

int main() {
    int n;
    cout << "Enter n (>= 10): ";
    cin >> n;
    
    if (n < 10) {
        cout << "n must be >= 10\n";
        return 1;
    }
    
    vector<int> A(n);
    cout << "Enter " << n << " numbers: ";
    for (int &x : A) cin >> x;

    auto res = minMax(A, 0, n - 1);
    cout << "Min = " << res.first << ", Max = " << res.second << "\n";
    cout << "Comparisons (divide & conquer) = " << cmpCount << "\n";
    cout << "Comparisons (brute force) = " << 2LL * (n - 1) << "\n";
    return 0;
}
