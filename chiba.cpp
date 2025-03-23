#include <iostream>
#include <vector>
#include <unordered_set>
#include <chrono>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <iomanip>

using namespace std;
using namespace chrono;


void readGraph(const string& filename, vector<unordered_set<int>>& graph, int& numVertices, vector<int>& vertexDegrees) {
    ifstream file(filename);
    string line;
    unordered_map<int, int> vertexMap;
    int vertexCount = 0;

    if (!file) {
        cerr << "Error: Unable to open file " << filename << endl;
        exit(1);
    }

    getline(file, line);
    stringstream ss(line);
    int numEdges;
    ss >> numVertices >> numEdges;

    while (getline(file, line)) {
        int u, v;
        if (line.empty()) continue;

        stringstream ss(line);
        ss >> u >> v;

        if (vertexMap.find(u) == vertexMap.end()) vertexMap[u] = vertexCount++;
        if (vertexMap.find(v) == vertexMap.end()) vertexMap[v] = vertexCount++;

        int uIndex = vertexMap[u];
        int vIndex = vertexMap[v];

        while (graph.size() <= max(uIndex, vIndex)) {
            graph.push_back(unordered_set<int>());
        }

        graph[uIndex].insert(vIndex);
        graph[vIndex].insert(uIndex);
    }

    vertexDegrees.resize(graph.size());
    for (int i = 0; i < graph.size(); ++i) {
        vertexDegrees[i] = graph[i].size();
    }

    numVertices = graph.size();
}


void update(int i, set<int>& C, const vector<unordered_set<int>>& graph,
            vector<int>& S, vector<int>& T, set<set<int>>& maximalCliques,
            const vector<int>& sortedVertices, int n) {
    if (i == n) {
        bool isClique = true;
        for (auto u = C.begin(); u != C.end(); ++u) {
            for (auto v = next(u); v != C.end(); ++v) {
                if (graph[*u].find(*v) == graph[*v].end()) {
                    isClique = false;
                    break;
                }
            }
            if (!isClique) break;
        }

        if (isClique && C.size() >= 2) {
            bool isMaximal = true;
            for (int v = 0; v < n; ++v) {
                if (C.count(v) == 0) {
                    bool connectedToAll = true;
                    for (auto u : C) {
                        if (graph[v].find(u) == graph[v].end()) {
                            connectedToAll = false;
                            break;
                        }
                    }
                    if (connectedToAll) {
                        isMaximal = false;
                        break;
                    }
                }
            }

            if (isMaximal) {
                maximalCliques.insert(C);
            }
        }
    } else {
        bool prune = false;
        for (auto u = C.begin(); u != C.end(); ++u) {
            if (graph[*u].find(sortedVertices[i]) == graph[*u].end()) {
                prune = true;
                break;
            }
        }

        if (!prune) {
            if (C.count(sortedVertices[i]) == 0) {
                C.insert(sortedVertices[i]);
                update(i + 1, C, graph, S, T, maximalCliques, sortedVertices, n);
                C.erase(sortedVertices[i]);
            }
        }

        update(i + 1, C, graph, S, T, maximalCliques, sortedVertices, n);
    }
}

void findCliques(const vector<unordered_set<int>>& graph, int numVertices, const vector<int>& vertexDegrees) {
    set<int> C;
    vector<int> S(numVertices, 0);
    vector<int> T(numVertices, 0);
    set<set<int>> maximalCliques;

    vector<int> sortedVertices(numVertices);
    iota(sortedVertices.begin(), sortedVertices.end(), 0);
    sort(sortedVertices.begin(), sortedVertices.end(),
         [&vertexDegrees](int a, int b) { return vertexDegrees[a] > vertexDegrees[b]; });

    auto start = high_resolution_clock::now();
    update(0, C, graph, S, T, maximalCliques, sortedVertices, numVertices);
    auto end = high_resolution_clock::now();
    duration<double> duration = end - start;

    cout << scientific << setprecision(6);
    cout << "Execution Time: " << duration.count() << " seconds" << endl;

    int largestCliqueSize = 0;
    for (const auto& clique : maximalCliques) {
        largestCliqueSize = max(largestCliqueSize, (int)clique.size());
    }

    cout << "Largest Clique Size: " << largestCliqueSize << endl;
    cout << "Total Number of Maximal Cliques: " << maximalCliques.size() << endl;

   
    map<int, int> maximalCliqueSizeDistribution;
    for (const auto& clique : maximalCliques) {
        if (clique.size() >= 2)
            maximalCliqueSizeDistribution[clique.size()]++;
    }

    cout << "Distribution of Clique Sizes (Maximal Cliques only, size ≥ 2):" << endl;
    for (const auto& entry : maximalCliqueSizeDistribution) {
        cout << "Size " << entry.first << ": " << entry.second << " cliques" << endl;
    }
}

int main() {
    vector<unordered_set<int>> graph;
    int numVertices;
    vector<int> vertexDegrees;

    string filename = "test4.txt";

    readGraph(filename, graph, numVertices, vertexDegrees);
    findCliques(graph, numVertices, vertexDegrees);

    return 0;
}


