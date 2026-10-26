#include <bits/stdc++.h>
using namespace std;


vector<long long> lltovec(long long x, long long base) {
    if (base <= 1) throw invalid_argument("base must be greater than 1");
    if (x < 0) throw invalid_argument("x must be non-negative");
    if(x == 0) return {0};
    vector<long long> ret;
    while(x > 0) {
        ret.emplace_back(x % base);
        x /= base;
    }
    reverse(ret.begin(), ret.end());
    return ret;
}

long long vectoll(const vector<long long> &x, long long base) {
    if (base <= 1) throw invalid_argument("base must be greater than 1");
    long long ret = 0;
    for(auto d : x) {
        if (d < 0 || d >= base) throw invalid_argument("invalid digit");
        if (ret > (LLONG_MAX - d) / base) {
            throw out_of_range("number does not fit in long long");
        }
        ret = ret * base + d;
    }
    return ret;
}

string lltostr(long long x, int base) {
    if (base < 2 || base > 36) throw invalid_argument("base must be in [2, 36]");
    if (x == 0) return "0";

    const bool negative = x < 0;
    unsigned long long magnitude = static_cast<unsigned long long>(x);
    if (negative) magnitude = -magnitude;

    string s;
    while(magnitude > 0) {
        int d = static_cast<int>(magnitude % static_cast<unsigned int>(base));
        if(d < 10) s += static_cast<char>('0' + d);
        else s += static_cast<char>('A' + (d - 10));
        magnitude /= base;
    }
    if (negative) s += '-';
    reverse(s.begin(), s.end());
    return s;
}

long long strtoll(const string& s, int base) {
    if (base < 2 || base > 36) throw invalid_argument("base must be in [2, 36]");
    if (s.empty()) throw invalid_argument("empty number");

    size_t pos = 0;
    bool negative = false;
    if (s[pos] == '-' || s[pos] == '+') {
        negative = s[pos] == '-';
        if (++pos == s.size()) throw invalid_argument("sign without digits");
    }

    const unsigned long long limit = negative
        ? static_cast<unsigned long long>(LLONG_MAX) + 1ULL
        : static_cast<unsigned long long>(LLONG_MAX);
    unsigned long long ret = 0;
    for (; pos < s.size(); ++pos) {
        const char c = s[pos];
        int d = -1;
        if ('0' <= c && c <= '9') d = c - '0';
        else if ('A' <= c && c <= 'Z') d = c - 'A' + 10;
        else if ('a' <= c && c <= 'z') d = c - 'a' + 10;
        if (d < 0 || d >= base) throw invalid_argument("invalid digit");
        if (ret > (limit - static_cast<unsigned long long>(d)) / base) {
            throw out_of_range("number does not fit in long long");
        }
        ret = ret * base + d;
    }

    if (!negative) return static_cast<long long>(ret);
    if (ret == static_cast<unsigned long long>(LLONG_MAX) + 1ULL) return LLONG_MIN;
    return -static_cast<long long>(ret);
}

int main() {
    long long a = 65530;
    string b = lltostr(a, 16);

    for(size_t i=0; i<b.size(); i++) {
        cout << b[i] << " ";
    }
    cout << endl;

    long long c = strtoll(b, 16);
    cout << c << endl;
}