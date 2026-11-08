#!/bin/bash
# bench.sh - optional timing script (Linux / macOS with GNU date / WSL).
# Builds routePlanner and gen_graph, generates random connected graphs, and
# prints END-TO-END wall-clock seconds (process start + file loading + search).
# These are measured times; they are NOT the theoretical Big O bounds.
set -e
g++ -std=c++17 -O2 main.cpp -o routePlanner
g++ -std=c++17 -O2 gen_graph.cpp -o gen_graph
T=$(mktemp -d)
now() { date +%s.%N; }
echo "== Option 2 (BFS shortest route), E = 3V =="
for V in 1000 10000 100000 1000000; do
  ./gen_graph $V $((3*V)) 7 > $T/g.txt
  s=$(now); printf "2\nCity_1\nCity_$V\n0\n" | ./routePlanner $T/g.txt > /dev/null; e=$(now)
  l0=$(now); printf "0\n" | ./routePlanner $T/g.txt > /dev/null; l1=$(now)
  echo "V=$V E=$((3*V)) option2=$(echo "$e - $s" | bc)s load_only=$(echo "$l1 - $l0" | bc)s"
done
echo "== Option 5 (exact round trip), E = 3K, start City_1 =="
for K in 10 12 14 16 18; do
  ./gen_graph $K $((3*K)) 5 > $T/g.txt
  s=$(now); r=$(printf "5\nCity_1\n0\n" | ./routePlanner $T/g.txt | grep -E "Total|No route" | head -1); e=$(now)
  echo "K=$K E=$((3*K)) time=$(echo "$e - $s" | bc)s result: $r"
done
rm -rf $T
