#include <iostream>
#include <vector>
#include <algorithm>
#include <cfloat>
#include <climits>
using namespace std;

double closest(vector<double>& a, int l, int r) {
    if (l >= r) {
        return DBL_MAX;
    }
    int m = (l + r) / 2;
    nth_element(a.begin() + l, a.begin() + m, a.begin() + r + 1);
    double d1 = closest(a, l, m);
    double d2 = closest(a, m + 1, r);
    double minS2 = *min_element(a.begin() + m + 1, a.begin() + r + 1);
    double maxS1 = a[m];
    return min({d1, d2, minS2 - maxS1});
}

int main() {
    int n;
    cout << "Enter n (>= 10): "; cin >> n;
    if (n < 10) { 
        cout << "n must be >= 10\n"; 
        return 1; 
    }
    vector<double> a(n);
    cout << "Enter " << n << " real numbers: ";
    for (double &x : a) cin >> x;
    cout << "Smallest distance between two numbers = " << closest(a, 0, n - 1) << "\n";
    return 0;
}
