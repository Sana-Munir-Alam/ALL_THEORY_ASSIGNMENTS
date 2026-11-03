#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
using namespace std;

double closest_1d(vector<double> arr)
{
    int n = arr.size();

    if (n < 2)
        return numeric_limits<double>::infinity();

    if (n == 2)
        return abs(arr[0] - arr[1]);

    vector<double> sorted = arr;
    sort(sorted.begin(), sorted.end());

    double m = sorted[n / 2];  // Median

    vector<double> left, right;

    for (double x : arr)
    {
        if (x < m)
            left.push_back(x);
        else
            right.push_back(x);
    }

    if (left.empty())
        return 0;

    double d = min(closest_1d(left), closest_1d(right));

    double cross = *min_element(right.begin(), right.end())
                 - *max_element(left.begin(), left.end());

    return min(d, cross);
}

void run_e()
{
    int n;

    cout << "Enter number of elements (minimum 10): ";
    cin >> n;

    while (n < 10)
    {
        cout << "Please enter at least 10 elements: ";
        cin >> n;
    }

    vector<double> a(n);

    cout << "Enter " << n << " array elements: ";

    for (int i = 0; i < n; i++)
        cin >> a[i];

    cout << "\nArray: ";

    for (double x : a)
        cout << x << " ";

    cout << endl;

    vector<double> s = a;
    sort(s.begin(), s.end());

    double check = numeric_limits<double>::infinity();

    for (int i = 0; i < n - 1; i++)
        check = min(check, s[i + 1] - s[i]);

    cout << "Smallest distance (divide & conquer) = "
         << closest_1d(a)
         << " | sort-and-scan check = " << check << endl;

    cout << "Efficiency: T(n) = 2T(n/2) + O(n) -> "
         << "Theta(n log n). Sorting + scanning is also "
         << "Theta(n log n) and simpler." << endl;
}

int main()
{
    run_e();
    return 0;
}
