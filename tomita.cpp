#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <map>
#include <iomanip>
#include <unordered_set>

using namespace std;

const int MAX_VERTICES = 100000;

class Graph {
private:
    vector<unordered_set<int>> adj;
    int n;

public:
    Graph(int num_vertices) : n(num_vertices) {
        adj.resize(n);
    }

    void addEdge(int u, int v) {
        if (u >= n || v >= n) {
            int new_size = max(u, v) + 1;
            adj.resize(new_size);
            n = new_size;
        }
        adj[u].insert(v);
        adj[v].insert(u);  
    }

    bool isAdjacent(int u, int v) const {
        if (u >= n || v >= n) return false;
        return adj[u].find(v) != adj[u].end();
    }

    const unordered_set<int>& getNeighbors(int v) const {
        return adj[v];
    }

    int getNumVertices() const { return n; }
};

class MaximalCliqueEnumerator {
private:
    const Graph& graph;
    vector<int> Q; 
    long long clique_count;
    bool print_cliques;
    int largest_clique_size;
    map<int, int> clique_size_distribution;

    
    vector<bool> in_subg;
    vector<bool> in_cand;
    vector<bool> in_temp;

    // here i am Find pivot vertex that maximizes |CAND ∩ Γ(u)|
    int findPivot(const vector<int>& SUBG, const vector<int>& CAND) {
        int max_size = -1, pivot = -1;
        
        for (int u : SUBG) {
            int count = 0;
            const auto& neighbors = graph.getNeighbors(u);
            
            for (int v : CAND) {
                if (neighbors.find(v) != neighbors.end()) {
                    count++;
                }
            }
            
            if (count > max_size) {
                max_size = count;
                pivot = u;
            }
        }
        
        return pivot;
    }

   
    void EXPAND(vector<int>& SUBG, vector<int>& CAND, vector<int>& FINI) {
      
        if (SUBG.empty()) {
            clique_count++;
            largest_clique_size = max(largest_clique_size, (int)Q.size());
            clique_size_distribution[Q.size()]++;

            if (print_cliques) {
                cout << "Maximal clique " << clique_count << ": ";
                for (int v : Q) cout << v << " ";
                cout << endl;
            }
            return;
        }

        
        if (CAND.empty()) return;

        
        int u = findPivot(SUBG, CAND);
        
       
        vector<int> ext_candidates;
        const auto& u_neighbors = graph.getNeighbors(u);
        
        for (int q : CAND) {
            if (u_neighbors.find(q) == u_neighbors.end()) {
                ext_candidates.push_back(q);
            }
        }

     
        for (int q : ext_candidates) {
            
            Q.push_back(q);
            
           
            vector<int> SUBGq, CANDq, FINIq;
            const auto& q_neighbors = graph.getNeighbors(q);
            
            SUBGq.reserve(SUBG.size());
            CANDq.reserve(CAND.size());
            FINIq.reserve(FINI.size());
            
            for (int v : SUBG) {
                if (q_neighbors.find(v) != q_neighbors.end()) {
                    SUBGq.push_back(v);
                }
            }
            
            for (int v : CAND) {
                if (q_neighbors.find(v) != q_neighbors.end()) {
                    CANDq.push_back(v);
                }
            }
            
            for (int v : FINI) {
                if (q_neighbors.find(v) != q_neighbors.end()) {
                    FINIq.push_back(v);
                }
            }
            
          
            EXPAND(SUBGq, CANDq, FINIq);
            
           
            CAND.erase(remove(CAND.begin(), CAND.end(), q), CAND.end());
            FINI.push_back(q);
            
            
            Q.pop_back();
        }
    }

public:
    MaximalCliqueEnumerator(const Graph& g, bool print = false)
        : graph(g), clique_count(0), print_cliques(print), largest_clique_size(0) {
       
        in_subg.resize(g.getNumVertices(), false);
        in_cand.resize(g.getNumVertices(), false);
        in_temp.resize(g.getNumVertices(), false);
    }

    long long enumerate() {
        vector<int> SUBG, CAND, FINI;
        SUBG.reserve(graph.getNumVertices());
        CAND.reserve(graph.getNumVertices());
        FINI.reserve(graph.getNumVertices());
        
        for (int i = 0; i < graph.getNumVertices(); i++) {
            SUBG.push_back(i);
            CAND.push_back(i);
        }

        Q.clear();
        Q.reserve(graph.getNumVertices());
        clique_count = 0;
        largest_clique_size = 0;
        clique_size_distribution.clear();

        clock_t start = clock();
        EXPAND(SUBG, CAND, FINI);
        clock_t end = clock();

        double duration = double(end - start) / CLOCKS_PER_SEC;

        cout << "Largest Clique Size: " << largest_clique_size << endl;
        cout << "Total Number of Maximal Cliques: " << clique_count << endl;
        cout << "Execution Time: " << fixed << setprecision(6) << duration << " seconds" << endl;

        cout << "\nDistribution of Clique Sizes:" << endl;
        for (const auto& [size, count] : clique_size_distribution) {
            cout << "Size " << size << ": " << count << " cliques" << endl;
        }

        return clique_count;
    }
};

Graph readGraph(const string& filename) {
    ifstream file(filename);
    int numNodes, numEdges;

    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    file >> numNodes >> numEdges;
    Graph graph(numNodes);

    cout << "Reading graph with " << numNodes << " nodes and " << numEdges << " edges..." << endl;

   
    int progress_step = max(1, numEdges / 100);
    
    for (int i = 0; i < numEdges; ++i) {
        int u, v;
        file >> u >> v;
        graph.addEdge(u, v);
        
        if ((i + 1) % progress_step == 0) {
            cout << "\rProgress: " << (i + 1) * 100 / numEdges << "%" << flush;
        }
    }
    cout << "\rProgress: 100%" << endl;

    return graph;
}

Graph readLargeGraph(const string& filename) {
    ifstream file(filename);
    int numNodes, numEdges;

    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    file >> numNodes >> numEdges;
    Graph graph(numNodes);

    cout << "Reading large graph with " << numNodes << " nodes and " << numEdges << " edges..." << endl;
    cout << "This may take a while for very large graphs." << endl;

   
    int chunk_size = 1000000; 
    int chunks = (numEdges + chunk_size - 1) / chunk_size;
    
    for (int chunk = 0; chunk < chunks; chunk++) {
        int start = chunk * chunk_size;
        int end = min((chunk + 1) * chunk_size, numEdges);
        
        cout << "\rProcessing chunk " << (chunk + 1) << "/" << chunks << " (" 
             << (chunk * 100 / chunks) << "%)" << flush;
        
        
        if (chunk > 0) {
            file.clear();
            file.seekg(0);
            file >> numNodes >> numEdges; 
            for (int i = 0; i < start; i++) {
                int u, v;
                file >> u >> v;
            }
        }
        
        
        for (int i = start; i < end; i++) {
            int u, v;
            file >> u >> v;
            graph.addEdge(u, v);
        }
    }
    cout << "\rProcessing complete: 100%" << endl;

    return graph;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <graph_file> [print_cliques] [large]" << endl;
        return 1;
    }
    
    const string filename = argv[1];
    const bool print_cliques = (argc > 2 && string(argv[2]) == "print");
    const bool large_graph = (argc > 3 && string(argv[3]) == "large");
    
    Graph graph = large_graph ? readLargeGraph(filename) : readGraph(filename);
    cout << "Graph loaded with " << graph.getNumVertices() << " vertices." << endl;
    
  
    if (graph.getNumVertices() > 50000) {
        cout << "Warning: Graph is very large. The algorithm may take a long time or run out of memory." << endl;
        cout << "Consider using a sampling approach or a distributed algorithm for large graphs." << endl;
        cout << "Do you want to continue? (y/n): ";
        char response;
        cin >> response;
        if (response != 'y' && response != 'Y') {
            cout << "Exiting program." << endl;
            return 0;
        }
    }

    MaximalCliqueEnumerator enumerator(graph, print_cliques);
    enumerator.enumerate();

    return 0;
}
