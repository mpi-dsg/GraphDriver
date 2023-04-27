#include <cstdint>
#include <string>
#include <unordered_map>
#include <fstream>

using namespace std;

// Set the current position of the handle at the start of the given section
enum class Section { VTX_FINAL, VTX_TEMP, EDGES };
void set_marker(const unordered_map<string, string>& properties, fstream& handle, Section section);

// Loader of whole blocks of edges
class EdgeLoader {
    EdgeLoader(const EdgeLoader&) = delete;
    EdgeLoader& operator=(const EdgeLoader&) = delete;

    fstream& m_handle; // handle to read the edges from the file
    static constexpr uint64_t m_input_stream_sz = (1ull << 20); // 256 KB
    uint8_t* m_input_stream { nullptr }; // compressed content read from the handle
    streampos m_input_stream_pos { 0 }; // offset of the next chunk to read the file

public:
    // Initialise the reader, the handle should already be positioned at the start of the compressed section
    EdgeLoader(fstream& handle);

    // Destructor
    ~EdgeLoader();

    // Load a whole block of edges in the given buffer. Return the number of edges loaded, or 0 if the array is not big enough to load the whole block
    uint64_t load(uint64_t* array, uint64_t num_edges);
};

// Iterate over an edge at the time from a block of edges
class EdgeBlockReader {
    shared_ptr<uint64_t[]> m_ptr_block; // the block of edges
    uint64_t m_position; // the current position in the block
    uint64_t m_num_edges; // the total number of edges in the current block

public:
    // Create a dummy reader
    EdgeBlockReader();

    // Create a new instance to iterate over of a block of edges, as retrieved from the EdgeLoader
    EdgeBlockReader(shared_ptr<uint64_t[]> block, uint64_t num_edges);

    // Copy ctor and assignment are allowed in this case
    EdgeBlockReader(const EdgeBlockReader&) = default;
    EdgeBlockReader& operator=(const EdgeBlockReader&) = default;

    // Destructor
    ~EdgeBlockReader();

    // Check whether there are more edges to read
    bool has_next() const;

    // Read one edge at the time
    bool read_edge(uint64_t& source, uint64_t& dest, double& weight);
};

// Read one block of edges at the time
class EdgeBlockLoader {
    EdgeBlockLoader(const EdgeBlockLoader&) = delete;
    EdgeBlockLoader& operator=(const EdgeBlockLoader&) = delete;

    EdgeLoader m_loader;
    const uint64_t m_max_num_edges; // max number of edges that can be stored in the buffer
    shared_ptr<uint64_t[]> m_ptr_buffer = 0; // pointer to where the edges are stored

public:
    // Create a new reader. The handle should be already positioned at the start of the compressed stream. The value for max_num_edges
    // should be the same of value of the property `internal.edges.block_size'
    EdgeBlockLoader(fstream& handle, uint64_t block_size_bytes);

    // Destructor
    ~EdgeBlockLoader();

    // Load one block of edges from the file
    EdgeBlockReader load();
};

class LogReader {
    LogReader(const LogReader&) = delete;
    LogReader& operator=(const LogReader&) = delete;

    fstream m_handle;
    EdgeBlockLoader m_loader;
    EdgeBlockReader m_reader;
public:
    LogReader(const string& path);
    ~LogReader();
    bool read_edge(uint64_t& src, uint64_t& des, double& weight);

private:
    unordered_map<string, string> parse_properties(const string& path);
    void print_properties(unordered_map<string, string> properties);
};