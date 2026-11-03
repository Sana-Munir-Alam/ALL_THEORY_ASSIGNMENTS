#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

long long sortCount(vector<long long>& a, int l, int r) {
    if (l >= r) return 0;
    int m = (l + r) / 2;
    long long cnt = sortCount(a, l, m) + sortCount(a, m + 1, r);
    int p = l;
    for (int j = m + 1; j <= r; j++) {
        while (p <= m && a[p] <= 2 * a[j]) p++;
        cnt += m - p + 1;
    }
    inplace_merge(a.begin() + l, a.begin() + m + 1, a.begin() + r + 1);
    return cnt;
}

int main() {
    int n;
    cout << "Enter n (>= 10): "; cin >> n;
    if (n < 10) { cout << "n must be >= 10\n"; return 1; }
    vector<long long> a(n);
    cout << "Enter " << n << " numbers: ";
    for (auto &x : a) cin >> x;
    cout << "Significant inversions = " << sortCount(a, 0, n - 1) << "\n";
    return 0;
}