#include <iostream>
#include <vector>
#include <climits>
using namespace std;

int main() {
    int n;
    cout << "Enter n (>= 10): "; cin >> n;

    if (n < 10) { cout << "n must be >= 10\n"; return 1; }
    vector<int> A(n);
    cout << "Enter " << n << " distinct numbers (unimodal: increasing then decreasing): ";
    for (int &x : A) cin >> x;
    
    int lo = 0, hi = n - 1, probes = 0, peak = -1;
    while (lo <= hi) {
        int m = lo + (hi - lo) / 2;
        int cur = A[m]; probes++;
        int left = (m - 1 >= lo) ? (probes++, A[m - 1]) : INT_MIN;
        int right = (m + 1 <= hi) ? (probes++, A[m + 1]) : INT_MIN;
        
        if (cur > left && cur > right) { peak = m; break; }
        else if (left < cur && cur < right) lo = m + 1;
        else hi = m - 1;
    }
    
    cout << "Peak is A[" << peak + 1 << "] = " << A[peak] << " (1-based), probes = " << probes << "\n";
    return 0;
}