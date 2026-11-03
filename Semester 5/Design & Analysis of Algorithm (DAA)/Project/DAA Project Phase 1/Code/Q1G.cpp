#include <iostream>
#include <vector>
using namespace std;

struct Res { 
    double best; 
    int buy, sell, mn, mx;
};

Res solve(const vector<double>& p, int l, int r) {
    if (l == r) return {-1e18, -1, -1, l, l};
    int m = (l + r) / 2;

    Res L = solve(p, l, m), R = solve(p, m + 1, r);
    Res out;

    out.best = L.best; out.buy = L.buy; out.sell = L.sell;

    if (R.best > out.best) { out.best = R.best; out.buy = R.buy; out.sell = R.sell; }
    double cross = p[R.mx] - p[L.mn];

    if (cross > out.best) { out.best = cross; out.buy = L.mn; out.sell = R.mx; }
    out.mn = (p[L.mn] <= p[R.mn]) ? L.mn : R.mn;
    out.mx = (p[L.mx] >= p[R.mx]) ? L.mx : R.mx;
    return out;
}
int main() {
    int n;
    cout << "Enter number of days n (>= 10): "; cin >> n;
    if (n < 10) { cout << "n must be >= 10\n"; return 1; }

    vector<double> p(n);
    cout << "Enter " << n << " prices: ";
    
    for (double &x : p) cin >> x;
    Res r = solve(p, 0, n - 1);
    
    if (r.best <= 0) cout << "No way to make money in this period.\n";
    else cout << "Buy on day " << r.buy + 1 << ", sell on day " << r.sell + 1 << ", profit per share = " << r.best << "\n";
    return 0;
}