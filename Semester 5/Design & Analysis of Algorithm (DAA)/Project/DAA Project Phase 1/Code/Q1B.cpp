#include <iostream>
using namespace std;

long long mults = 0;

long long power(long long a, int n) {
    if (n == 1) return a;
    long long t = power(a, n / 2);
    mults++;
    long long sq = t * t;
    if (n % 2 == 0) return sq;
    mults++;
    return sq * a;
}

int main() {
    long long a;
    int n;
    cout << "Enter base a and exponent n (n >= 10): ";
    cin >> a >> n;

    if (n < 10) {
        cout << "n must be >= 10\n";
        return 1;
    }
    
    long long result = power(a, n);
    cout << a << "^" << n << " = " << result << "\n";
    cout << "Multiplications (D&C) = " << mults << "\n";
    cout << "Multiplications (brute force) = " << n - 1 << "\n";
    cout << "(Keep a^n within the range of long long)\n";
    return 0;
}
