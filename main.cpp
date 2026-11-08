// Road Trip Route Planner - CSCE 2110.501, Team 6.
// Loads a road graph and handles shortest routes, stops, round trips, and meetups.
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

class Graph {
    vector<string> names;
    unordered_map<string, int> ids;
    vector<vector<int>> neighbors;
    vector<unordered_map<int, string>> roadNames;

    struct Search {
        vector<int> distance;
        vector<int> parent;
    };

    int cityId(const string& name) const {
        auto entry = ids.find(name);
        if (entry == ids.end())
            throw runtime_error("Unknown city: " + name + ". Names are case-sensitive.");
        return entry->second;
    }

    // The first visit to a city in BFS uses the fewest road connections.
    Search bfs(int start) const {
        Search result{vector<int>(names.size(), -1), vector<int>(names.size(), -1)};
        queue<int> pending;
        result.distance[start] = 0;
        pending.push(start);
        while (!pending.empty()) {
            int current = pending.front();
            pending.pop();
            for (int next : neighbors[current]) {
                if (result.distance[next] != -1) continue;
                result.distance[next] = result.distance[current] + 1;
                result.parent[next] = current;
                pending.push(next);
            }
        }
        return result;
    }

    // Follow the parent links backward, then reverse the result.
    vector<string> pathTo(const Search& search, int goal) const {
        if (search.distance[goal] == -1) return {};
        vector<string> route;
        for (int city = goal; city != -1; city = search.parent[city])
            route.push_back(names[city]);
        reverse(route.begin(), route.end());
        return route;
    }

public:
    // Format: V E, then V unique names, then E pairs of city names.
    // City names must be single words (use underscores for spaces).
    void load(const string& filename) {
        ifstream input(filename);
        if (!input) throw runtime_error("Cannot open input file: " + filename);
        int cityCount, roadCount;
        if (!(input >> cityCount >> roadCount) || cityCount < 0 || roadCount < 0)
            throw runtime_error("Expected nonnegative city and road counts.");
        names.clear(); ids.clear(); neighbors.assign(cityCount, {});
        roadNames.assign(cityCount, {});
        for (int i = 0; i < cityCount; ++i) {
            string name;
            if (!(input >> name)) throw runtime_error("Missing city name.");
            if (ids.count(name)) throw runtime_error("Duplicate city name: " + name);
            ids[name] = i;
            names.push_back(name);
        }
        for (int i = 0; i < roadCount; ++i) {
            string from, to;
            if (!(input >> from >> to)) throw runtime_error("Missing road endpoint.");
            if (!ids.count(from) || !ids.count(to))
                throw runtime_error("Road contains an unlisted city.");
            string road;
            getline(input, road);
            size_t beginning = road.find_first_not_of(" \t\r");
            road = beginning == string::npos ? "Unspecified test connection" : road.substr(beginning);
            int a = ids.at(from), b = ids.at(to);
            if (a == b) throw runtime_error("Self-loop roads are not supported.");
            // Ignore repeated roads, including pairs listed in reverse order.
            if (find(neighbors[a].begin(), neighbors[a].end(), b) == neighbors[a].end()) {
                neighbors[a].push_back(b);
                neighbors[b].push_back(a);
                roadNames[a][b] = road;
                roadNames[b][a] = road;
            }
        }
        string extra;
        if (input >> extra) throw runtime_error("Unexpected data after the listed roads.");
    }

    void print() const {
        cout << "\nCities and direct road connections:\n";
        for (size_t i = 0; i < names.size(); ++i) {
            cout << names[i] << ":";
            if (neighbors[i].empty()) cout << " (no connections)";
            for (int next : neighbors[i]) cout << "\n  " << names[next] << " via " << roadNames[i].at(next);
            cout << '\n';
        }
    }

    void printRoads(const vector<string>& route) const {
        for (size_t i = 1; i < route.size(); ++i) {
            int from = cityId(route[i - 1]), to = cityId(route[i]);
            cout << "  " << route[i - 1] << " -> " << route[i]
                 << " via " << roadNames[from].at(to) << '\n';
        }
    }

    vector<string> shortestRoute(const string& from, const string& to) const {
        int start = cityId(from), goal = cityId(to);
        return pathTo(bfs(start), goal);
    }

    // Compare both stop orders using shortest paths for each leg.
    vector<string> routeVia(const string& from, const string& first,
                            const string& second, const string& to) const {
        int start = cityId(from), a = cityId(first), b = cityId(second), goal = cityId(to);
        Search fromStart = bfs(start), fromA = bfs(a), fromB = bfs(b);
        if (fromStart.distance[a] < 0 || fromStart.distance[b] < 0 ||
            fromStart.distance[goal] < 0) return {};

        long long aFirst = static_cast<long long>(fromStart.distance[a]) +
                           fromA.distance[b] + fromB.distance[goal];
        long long bFirst = static_cast<long long>(fromStart.distance[b]) +
                           fromB.distance[a] + fromA.distance[goal];
        if (bFirst < aFirst) {
            swap(a, b);
            swap(fromA, fromB);
        }
        vector<string> route = pathTo(fromStart, a);
        vector<string> middle = pathTo(fromA, b);
        vector<string> end = pathTo(fromB, goal);
        route.insert(route.end(), next(middle.begin()), middle.end());
        route.insert(route.end(), next(end.begin()), end.end());
        return route;
    }

    struct Meeting {
        string city;
        vector<vector<string>> routes;
        long long totalConnections = 0;
    };

    // Score every city that all three travelers can reach.
    Meeting meetingCity(const string& first, const string& second,
                        const string& third) const {
        int a = cityId(first), b = cityId(second), c = cityId(third);
        if (a == b || a == c || b == c)
            throw runtime_error("Enter three different starting cities.");
        Search fromA = bfs(a), fromB = bfs(b), fromC = bfs(c);
        Meeting best;
        int bestId = -1;
        for (int city = 0; city < static_cast<int>(names.size()); ++city) {
            if (city == a || city == b || city == c) continue;
            int da = fromA.distance[city], db = fromB.distance[city], dc = fromC.distance[city];
            if (da < 0 || db < 0 || dc < 0) continue;
            long long total = static_cast<long long>(da) + db + dc;
            if (bestId == -1 || total < best.totalConnections) {
                bestId = city;
                best.totalConnections = total;
            }
        }
        if (bestId != -1) {
            best.city = names[bestId];
            best.routes = {pathTo(fromA, bestId), pathTo(fromB, bestId), pathTo(fromC, bestId)};
        }
        return best;
    }

    // Exact BFS on (current city, set of visited cities). The mask stores one
    // bit for each city reachable from the start. A return to start ends a trip.
    vector<string> roundTrip(const string& from) const {
        int start = cityId(from);
        Search reachable = bfs(start);
        vector<int> component{start};
        vector<int> localId(names.size(), -1);
        localId[start] = 0;
        for (int city = 0; city < static_cast<int>(names.size()); ++city) {
            if (city == start || reachable.distance[city] == -1) continue;
            localId[city] = static_cast<int>(component.size());
            component.push_back(city);
        }
        int count = static_cast<int>(component.size());
        if (count == 1) return {from};
        constexpr int maxRoundTripCities = 18;
        if (count > maxRoundTripCities)
            throw runtime_error("Exact round trips support up to 18 reachable cities.");

        int maskCount = 1 << count;
        int fullMask = maskCount - 1;
        // Encode the pair as mask * count + local city ID.
        int initial = count;
        int goal = fullMask * count;
        vector<int> parent(maskCount * count, -1);
        queue<int> pending;
        parent[initial] = initial;
        pending.push(initial);
        while (!pending.empty() && parent[goal] == -1) {
            int state = pending.front();
            pending.pop();
            int mask = state / count;
            int current = component[state % count];
            for (int neighbor : neighbors[current]) {
                if (neighbor == start && mask != fullMask) continue;
                int nextCity = localId[neighbor];
                int nextMask = mask | (1 << nextCity);
                int nextState = nextMask * count + nextCity;
                if (parent[nextState] != -1) continue;
                parent[nextState] = state;
                pending.push(nextState);
            }
        }
        if (parent[goal] == -1) return {};
        vector<string> route;
        for (int state = goal; state != initial; state = parent[state])
            route.push_back(names[component[state % count]]);
        route.push_back(from);
        reverse(route.begin(), route.end());
        return route;
    }
};

// Parse a whole line, rejecting inputs such as "2abc".
bool parseInteger(const string& line, int& value) {
    istringstream input(line);
    string extra;
    return (input >> value) && !(input >> extra);
}

// Returns false if the input stream closes during a menu operation.
bool prompt(const string& label, string& answer) {
    cout << label;
    return static_cast<bool>(getline(cin, answer));
}

void printRoute(const Graph& graph, const vector<string>& route) {
    if (route.empty()) {
        cout << "No route exists.\n";
        return;
    }
    cout << "Route: ";
    for (size_t i = 0; i < route.size(); ++i) {
        if (i) cout << " -> ";
        cout << route[i];
    }
    cout << "\nTotal connections: " << route.size() - 1 << '\n';
    graph.printRoads(route);
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        cerr << "Usage: routePlanner [input-file]\n";
        return 1;
    }
    Graph graph;
    try {
        graph.load(argc == 2 ? argv[1] : "cities.txt");
    } catch (const exception& error) {
        cerr << "Input error: " << error.what() << '\n';
        return 1;
    }
    cout << "Road Trip Route Planner\n";
    while (true) {
        cout << "\n1. Show cities and roads\n2. Find shortest route\n"
             << "3. Find route with fewer than x connections\n"
             << "4. Find route through two required cities\n"
             << "5. Find shortest round trip\n6. Find meeting city\n0. Exit\n";
        string line;
        if (!prompt("Choice: ", line)) break;
        int choice;
        if (!parseInteger(line, choice) || choice < 0 || choice > 6) {
            cout << "Enter a menu number from 0 to 6.\n";
            continue;
        }
        if (choice == 0) break;
        if (choice == 1) {
            graph.print();
            continue;
        }
        try {
            string from, to, first, second;
            if (choice == 6) {
                if (!prompt("Traveler 1 starting city: ", from) ||
                    !prompt("Traveler 2 starting city: ", first) ||
                    !prompt("Traveler 3 starting city: ", second)) break;
                Graph::Meeting result = graph.meetingCity(from, first, second);
                if (result.city.empty()) {
                    cout << "No eligible meeting city exists.\n";
                } else {
                    cout << "Meeting city: " << result.city << '\n';
                    for (size_t i = 0; i < result.routes.size(); ++i) {
                        cout << "Traveler " << i + 1 << " - ";
                        printRoute(graph, result.routes[i]);
                    }
                    cout << "Combined connections: " << result.totalConnections << '\n';
                }
                continue;
            }
            if (!prompt("Starting city: ", from)) break;
            if (choice == 5) {
                printRoute(graph, graph.roundTrip(from));
                continue;
            }
            if (!prompt("Destination: ", to)) break;
            if (choice == 4) {
                if (!prompt("Required city 1: ", first) ||
                    !prompt("Required city 2: ", second)) break;
                printRoute(graph, graph.routeVia(from, first, second, to));
                continue;
            }
            int limit = 0;
            if (choice == 3) {
                if (!prompt("Connection limit x (strictly fewer than x): ", line)) break;
                if (!parseInteger(line, limit) || limit < 0) {
                    cout << "Enter a nonnegative integer limit.\n";
                    continue;
                }
            }
            vector<string> route = graph.shortestRoute(from, to);
            if (choice == 3 && !route.empty() && route.size() - 1 >= static_cast<size_t>(limit)) {
                cout << "No route has fewer than " << limit << " connections.\n"
                     << "Shortest available route requires " << route.size() - 1 << ".\n";
            } else {
                printRoute(graph, route);
            }
        } catch (const exception& error) {
            cout << error.what() << '\n';
        }
    }
    return 0;
}
