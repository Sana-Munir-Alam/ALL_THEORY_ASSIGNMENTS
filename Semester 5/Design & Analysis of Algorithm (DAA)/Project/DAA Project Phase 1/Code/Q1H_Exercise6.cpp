#include <iostream>
#include <vector>
using namespace std;

vector<int> x;
int n, probes = 0;
int probe(int v) { probes++; return x[v]; }

int main() {
    cout << "Enter n = 2^d - 1 (n >= 15): "; cin >> n;
    if (n < 15 || ((n + 1) & n) != 0) { 
        cout << "n must be 2^d - 1 and >= 15\n";
        return 1; 
    }
    x.assign(n + 1, 0);
    cout << "Enter " << n << " distinct labels (level order): ";
    for (int i = 1; i <= n; i++) {
        cin >> x[i];
    }
    int v = 1, xv = probe(1);
    while (true) {
        if (2 * v > n) break;
        int xl = probe(2 * v), xr = probe(2 * v + 1);
        if (xv < xl && xv < xr) 
            break;
        if (xl < xr) { 
            v = 2 * v; xv = xl; 
        } else { 
            v = 2 * v + 1; xv = xr; 
        }
    }
    cout << "Local minimum: node " << v << " with label " << xv << ", probes = " << probes << "\n";
    return 0;
}
