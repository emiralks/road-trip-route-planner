// gen_graph.cpp - optional helper that writes a random connected test graph
// in the same input format as cities.txt (Team 6, CSCE 2110.501).
// Usage: ./gen_graph <num_cities> <num_roads> [seed] > output.txt
// A random spanning tree is built first (so the graph is connected), then
// extra random roads are added until <num_roads> distinct roads exist.
#include <cstdlib>
#include <iostream>
#include <random>
#include <set>
#include <utility>
#include <vector>
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: gen_graph <num_cities> <num_roads> [seed]\n";
        return 1;
    }
    long long v = atoll(argv[1]), e = atoll(argv[2]);
    unsigned seed = argc > 3 ? static_cast<unsigned>(atoi(argv[3])) : 1u;
    if (v < 2 || e < v - 1 || e > v * (v - 1) / 2) {
        cerr << "Need cities >= 2 and cities-1 <= roads <= cities*(cities-1)/2\n";
        return 1;
    }
    mt19937_64 rng(seed);
    set<pair<long long, long long>> roads;
    for (long long i = 1; i < v; ++i) {           // spanning tree
        long long j = static_cast<long long>(rng() % i);
        roads.insert({j, i});
    }
    while (static_cast<long long>(roads.size()) < e) {  // extra roads
        long long a = static_cast<long long>(rng() % v), b = static_cast<long long>(rng() % v);
        if (a == b) continue;
        roads.insert({min(a, b), max(a, b)});
    }
    cout << v << ' ' << e << '\n';
    for (long long i = 1; i <= v; ++i) cout << "City_" << i << '\n';
    for (const auto& r : roads)
        cout << "City_" << r.first + 1 << " City_" << r.second + 1 << " Test road\n";
    return 0;
}
