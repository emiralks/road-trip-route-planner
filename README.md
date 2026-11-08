# Road Trip Route Planner

CSCE 2110.501 — Foundations of Data Structures  
Team 6 — Yahya Baban and Emir Alakus

The C++ program reads a graph of cities and roads and answers route queries.
A city is a vertex and a road is an undirected edge. Each edge costs one
connection, so the program minimizes the number of roads traveled.

## Compile and run

Extract the ZIP. Open a terminal in the extracted folder containing
`main.cpp` and `cities.txt`. Use a C++17 compiler and the standard library.

Windows PowerShell, with g++ on PATH:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o routePlanner.exe
.\routePlanner.exe cities.txt
```

macOS or Linux, with g++ installed (clang++ also works):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o routePlanner
./routePlanner cities.txt
```

The program defaults to `cities.txt` if you omit the filename.
To try another dataset, replace `cities.txt` in the run command with
`cities_disconnected.txt` or `cities_large.txt`.

## Input and operations

The first line gives the number of cities and the number of road records.
It is followed by the city names and then road records, one per line, containing two city names followed by the highway label. For example:

```text
3 2
Dallas
Plano
Frisco
Dallas Plano Dallas North Tollway (TOLL)
Plano Frisco Dallas North Tollway (TOLL)
```

The default file has 15 North Texas city names and 19 selected real highway corridors.
Cities are collapsed into single vertices. A connection represents a highway
corridor between city areas and may pass through omitted municipalities.
Local streets and transfers between highways within a city are omitted.
This is a simplified corridor model, not complete turn-by-turn navigation. Distances, traffic, and driving time
are outside this model. Names are case-sensitive. Use underscores for spaces
(e.g., `Fort_Worth`). Repeated roads are ignored. Duplicate city names,
self-loops, unlisted endpoints, and invalid record counts are rejected.

| Menu | Operation |
|---|---|
| 1 | Print every city and its direct road connections |
| 2 | Find the route with the fewest connections between two cities |
| 3 | Find the shortest route if it uses strictly fewer than x connections |
| 4 | Find the shortest route visiting two specified cities in either order |
| 5 | Find the shortest round trip visiting every city reachable from the start |
| 6 | Find a meeting city for three distinct starting cities |
| 0 | Exit |

Required-stop and round-trip routes may revisit intermediate cities. A round
trip ends the first time it returns to the start. An isolated start yields a
zero-connection round trip. For a meeting, each traveler gets a route to the
eligible city with the lowest combined number of connections. The three
starting cities are excluded. Equal-distance BFS paths follow road order,
and tied meeting cities follow city order in the file.

## Data structures and complexity

The program maps city names to integer IDs with an unordered map, stores
names in a vector, and stores roads in a vector of neighbor lists. It uses
a FIFO queue for breadth-first search (BFS). A distance of -1 marks an
unvisited city. Parent pointers let the program reconstruct each route.

Let V be the number of cities and E the number of distinct roads. The graph
uses O(V + E) space. A BFS takes O(V + E) time and O(V) extra space, treating
city names as bounded-length labels. Menu options 2–4 and 6 use a constant
number of BFS traversals. Option 4 compares the sum of shortest path lengths
in each stop order. Option 6 runs three searches and scans the candidate cities.
For R input road records, linear checks for duplicate roads make loading take
O(V + R × V) in the worst case with expected constant-time hash lookups.

For the round trip, the search state stores the current city and a bitmask
of visited cities. Each move takes one road. A return to the starting city
is allowed only after every reachable city has been visited. Since BFS
explores states by number of moves, the first valid return is optimal.
If K cities and M roads are in the starting component, the full search
takes O(V + E + 2^K × (K + M)) time and O(V + K × 2^K) extra space.
Exact round trips are capped at 18 reachable cities to bound memory use.
The other operations have no such cap.

## Demo and test results

Enter the values in each row one at a time. Select 0 when finished.

| Input file | Menu and entries | Expected | Observed |
|---|---|---|---|
| cities.txt | 2, Dallas, Frisco | 2 connections | Dallas → Plano → Frisco, 2 |
| cities.txt | 3, Dallas, Frisco, 2 | Reject equality | No route under 2 |
| cities.txt | 3, Dallas, Frisco, 3 | Accept the 2-edge route | Dallas → Plano → Frisco, 2 |
| cities.txt | 4, Dallas, Frisco, McKinney, Allen | 4 connections, both stops | Dallas → Plano → Allen → McKinney → Frisco, 4 |
| cities.txt | 2, Allen, Denton | 3 connections | Allen → Plano → Frisco → Denton, 3 |
| cities.txt | 5, Allen | 15 cities covered, 19 connections | Route returns to Allen, 19 |
| cities.txt | 5, Plano | Valid round trip | Route returns to Plano, 19 |
| cities.txt | 5, Dallas | No valid trip under the return rule | No route exists |
| cities.txt | 2, Dallas, Dallas | 0 connections | Dallas, 0 |
| cities.txt | 6, Dallas, Dallas, Denton | Reject repeated start | Enter three different starting cities. |
| cities_disconnected.txt | 5, Dallas | Round trip inside its component | Dallas → Plano → Allen → Frisco → Denton → Dallas, 5 |
| cities.txt | 6, Dallas, Frisco, Allen | Plano, 3 combined | Plano, each traveler uses 1 connection |
| cities_disconnected.txt | 2, Dallas, Austin | No route | No route exists |
| cities_disconnected.txt | 6, Dallas, Frisco, Austin | No meeting city | No eligible meeting city exists |
| cities.txt | 2, Dallas, Fake_City | Unknown city message | Unknown city: Fake_City |
| cities_large.txt | 2, City_01, City_31 | 6 connections | 6 connections |
| cities_large.txt | 5, City_01 | Reachable-component size limit | Exact round trips support up to 18 reachable cities |

The three datasets exercise 6, 15, and 60 cities. Shortest routing worked
on all three sizes. The 60-city round trip reached the documented size limit.
Dallas is an articulation point separating the eastern branch in this selected
network. A tour covering the east and west would return to Dallas early, so
the program correctly reports no round trip from Dallas under the course rule.
Use Allen or Plano as the starting city for a feasible round-trip demonstration.

### Measured running times
`gen_graph.cpp` and `bench.sh` (both optional) generate random connected graphs
and time the program; our run is saved in `benchmark_results.txt`. Times are
measured end-to-end wall-clock seconds (process start, file loading and search)
on one Linux machine compiled with -O2; they are not the Big O bounds and will
differ on other computers. Run `./bench.sh` (Linux/macOS/WSL) to reproduce them.

| Option | Input size | Measured time |
|---|---|---|
| 2 (BFS) | V=1,000, E=3,000 | about 0.006 s |
| 2 (BFS) | V=10,000, E=30,000 | about 0.02 s |
| 2 (BFS) | V=100,000, E=300,000 | about 0.44 s |
| 2 (BFS) | V=1,000,000, E=3,000,000 | about 7.2 s |
| 5 (round trip) | K=10 / 12 / 14 | about 0.006 / 0.007 / 0.007 s |
| 5 (round trip) | K=16 / 18 | about 0.013 / 0.06 s |

Loading the file takes almost as long as the whole run (see `load_only` in
`benchmark_results.txt`), so file reading dominates the larger runs. The round
trip is fast on these small graphs, but its state space doubles with every extra
city, which is why it is capped at 18 reachable cities.

## Completed features and known limitations
All six menu options are implemented and tested. Known limitations: city
names must be single words (underscores for spaces) and are case-sensitive;
routes minimize the number of connections, not miles or time; exact round trips
are limited to 18 reachable cities; the round trip ends the first time it
returns to the start, so no round trip exists from a start city whose removal
disconnects the graph (for example Dallas in `cities.txt`); the default map
is a simplified city-level model.

## Files
`main.cpp` (program), `cities.txt`, `cities_disconnected.txt`, `cities_large.txt`
(input files), `round_trip_example_output.txt` (saved output of menu option 5
for Plano, not an input file), `gen_graph.cpp`, `bench.sh` and
`benchmark_results.txt` (optional scalability tests),
`Baban_Alakus_Report.pdf` / `.docx` (project report),
`Baban_Alakus_FinalProject_FullDeck.pptx` (complete slide deck) and
`screenshots/` (program output and code screenshots used in the slides).

The two additional test files contain synthetic connections with no highway
names. Older two-column road files remain supported and display
"Unspecified test connection". Real-road labels are in the default cities.txt.
Highway transfers within a city are abstracted into its vertex, so a displayed
city-to-city route omits local access roads. All highway directions, exits,
closures, mileage, and travel times are outside this project's model.
The optimization still minimizes graph connections.

## Road references
The selected corridor labels were checked against these official sources.
The city-level edges are our simplified interpretation of those corridors.

- NTTA, Dallas North Tollway — https://www.ntta.org/dallas-north-tollway-dnt
- NTTA, Plan Your Trip — https://www.ntta.org/plan-your-trip
- TxDOT, Dallas–Fort Worth toll roads and highway corridors — https://www.txdot.gov/discover/toll-roads-managed-lanes/txdot-toll-roads/dallas-fort-worth-area-toll-roads.html
- TxDOT, Dallas project fact sheets — https://www.txdot.gov/about/districts/dallas-district/fact-sheets.html
- TxDOT, I-35E Phase 1 — https://www.txdot.gov/projects/projects-studies/dallas/i35e-phase2/phase1.html
- City of Richardson, US 75 — https://www.cor.net/departments/transportation-mobility/us-75-tech-lane
- Federal Permitting Dashboard, US 380 Prosper/Frisco — https://www.permits.performance.gov/permitting-project/dot-projects/us-380-prosperfrisco

The feature definitions follow the course project materials and the team's
Road Trip Route Planner topic. AI tools (ChatGPT, and Claude for final
review and editing) were used to help write and test the code, the README and
the presentation. The team reviewed, compiled and tested the program, and
every output shown in the presentation was produced by this code. No code was
copied from other projects or websites; the standard C++ library
documentation (cppreference.com) was used for reference. Breadth-first search
and bitmask state search are standard textbook techniques from the course.

## GitHub repository
https://github.com/emiralks/road-trip-route-planner

## Team contribution
The project was completed by a team of two, and all work (design, coding,
testing, slides and documentation) was done together: Yahya Baban 50%,
Emir Alakus 50%.
