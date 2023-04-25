#include <string>
#include <memory>
#include <vector>
#include <fstream>

using namespace std;

struct edge_t {
    uint64_t src;
    uint64_t des;
};

class GraphReader {
    vector<edge_t> edges;
    size_t pos {0};
    
public:
    GraphReader(const string& path);
    ~GraphReader();

    bool read_edge(uint64_t& src, uint64_t& dst);
    bool read_vertex(uint64_t& vtx);

    uint64_t get_n_vertices(){ return n_vertices; }
    uint64_t get_n_edges(){ return n_edges; }

private:
    string vertex_file;
    string edge_file;
    uint64_t n_vertices;
    uint64_t n_edges;
    fstream handle_vertex_file;
    fstream handle_edge_file;
    void parse_properties(const string& path);
};