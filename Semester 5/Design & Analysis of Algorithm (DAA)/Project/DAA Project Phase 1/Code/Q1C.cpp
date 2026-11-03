#include <iostream>
#include <vector>
using namespace std;

long long mergeCount(vector<int>& A, vector<int>& T, int l, int m, int r) {
    long long inv = 0;
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) {
        if (A[i] <= A[j]) {
            T[k++] = A[i++];
        } else {
            T[k++] = A[j++];
            inv += m - i + 1;
        }
    }

    while (i <= m) T[k++] = A[i++];
    while (j <= r) T[k++] = A[j++];
    for (i = l; i <= r; i++) A[i] = T[i];
    return inv;
}

long long sortCount(vector<int>& A, vector<int>& T, int l, int r) {
    if (l >= r) return 0;
    int m = l + (r - l) / 2;
    long long leftInv = sortCount(A, T, l, m);
    long long rightInv = sortCount(A, T, m + 1, r);
    long long crossInv = mergeCount(A, T, l, m, r);
    return leftInv + rightInv + crossInv;
}

int main() {
    int n;
    cout << "Enter n (>= 10): ";
    cin >> n;

    if (n < 10) {
        cout << "n must be >= 10\n";
        return 1;
    }

    vector<int> A(n), T(n);

    cout << "Enter " << n << " numbers: ";
    for (int& x : A) cin >> x;
    cout << "Number of inversions = " << sortCount(A, T, 0, n - 1) << "\n";
    return 0;
}
