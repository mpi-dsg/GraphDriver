#include <string>
#include <fstream>
#include <regex>
#include "zlib.h"

#include "log_reader.hpp"
#include "../configuration.hpp"

using namespace std;

void set_marker(const unordered_map<string, string>& properties, fstream& handle, Section section){
    unordered_map<string, string>::const_iterator property;

    switch(section){
    case Section::VTX_FINAL:
        property = properties.find("internal.vertices.final.begin");
        break;
    case Section::VTX_TEMP:
        property = properties.find("internal.vertices.temporary.begin");
        break;
    case Section::EDGES:
        property = properties.find("internal.edges.begin");
        break;
    }

    // if(property == properties.end()) ERROR("Missing required property");

    handle.clear(); // if eof() -> clear the flags
    handle.seekg(stoull(property->second));
}

/*
    Edge Loader
*/
EdgeLoader::EdgeLoader(fstream& handle) : m_handle(handle) {
    m_input_stream = new uint8_t[m_input_stream_sz];
}

EdgeLoader::~EdgeLoader(){
    delete[] m_input_stream; m_input_stream = nullptr;
}

uint64_t EdgeLoader::load(uint64_t* array, uint64_t num_edges){
    if(!m_handle.good()) return 0;

    std::streampos handle_pos_start = m_handle.tellg();
    uint64_t num_edges_loaded = 0;
    bool done = false;

    // init the zlib stream
    z_stream z;
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;
    z.avail_in = 0;
    z.next_in = (unsigned char*) m_input_stream;
    uint64_t output_buffer_sz = num_edges * sizeof(uint64_t) * 3;
    z.avail_out = output_buffer_sz;
    z.next_out = (unsigned char*) array;
    int rc = inflateInit2(&z, -15);
    // if(rc != Z_OK) ERROR("Cannot initialise the library zlib");

    do {
        // the input buffer must have been consumed
        // assert(z.avail_in == 0);
        uint64_t input_stream_sz = 0;

        // read the content from the input file
        std::streampos handle_pos_last = m_handle.tellg();

        m_handle.read((char*) m_input_stream, m_input_stream_sz);
        input_stream_sz = m_input_stream_sz;
        if(m_handle.eof()){
            m_handle.clear();
            m_handle.seekg(0, ios_base::end);
            input_stream_sz = static_cast<uint64_t>(m_handle.tellg()) - handle_pos_last;
        } 
        // else if(m_handle.bad()){ ERROR("Cannot read from the input file"); }

        if(input_stream_sz == 0) break; // EOF, there is nothing to read

        z.next_in = m_input_stream;
        z.avail_in = input_stream_sz;

        // COUT_DEBUG("[INPUT EDGES] " << (int) m_input_stream[0] << ":" << (int) m_input_stream[1] << ":" << (int) m_input_stream[2] << ":" << (int) m_input_stream[3] << " z.avail_in: " << z.avail_in);

        // decompress the input
        rc = inflate(&z, Z_NO_FLUSH);
        // if(rc != Z_OK && rc != Z_STREAM_END) ERROR("Cannot decompress the input stream: rc: " << rc << ")");

        // there is not enough space to load the whole block in the buffer
        done = z.avail_out == 0 || rc == Z_STREAM_END;

        if(z.avail_out == 0 && rc != Z_STREAM_END){
            m_handle.seekg(handle_pos_start);
        } else if (rc == Z_STREAM_END){
            // assert((output_buffer_sz - z.avail_out) % (3 * sizeof(uint64_t)) == 0);
            num_edges_loaded = (output_buffer_sz - z.avail_out) / (3 * sizeof(uint64_t));
            m_handle.seekg(static_cast<int64_t>(handle_pos_last) + (input_stream_sz - z.avail_in), ios_base::beg);
        }
    } while (!done);

    inflateEnd(&z);
    return num_edges_loaded;
}


/*
    Edge Block Reader
*/
EdgeBlockReader::EdgeBlockReader() : m_ptr_block(), m_position(0), m_num_edges(0) {

}

EdgeBlockReader::EdgeBlockReader(shared_ptr<uint64_t[]> block, uint64_t num_edges) : m_ptr_block(block), m_position(0), m_num_edges(num_edges){

}

EdgeBlockReader::~EdgeBlockReader(){
    /* nop */
}

bool EdgeBlockReader::read_edge(uint64_t& source, uint64_t& dest, double& weight){
    if(m_position >= m_num_edges){
        return false;
    } else {
        uint64_t* __restrict sources = m_ptr_block.get();
        uint64_t* __restrict destinations = sources + m_num_edges;
        double* __restrict weights = reinterpret_cast<double*>(destinations + m_num_edges);

        source = sources[m_position];
        dest = destinations[m_position];
        weight = weights[m_position];

        m_position++;
        return true;
    }
}

bool EdgeBlockReader::has_next() const {
    return m_position < m_num_edges;
}

/*
    Edge Block Loader
*/
EdgeBlockLoader::EdgeBlockLoader(std::fstream& handle, uint64_t block_size_bytes) : m_loader(handle), m_max_num_edges(block_size_bytes / (3* sizeof(uint64_t))) {
    // if(block_size_bytes % (3*sizeof(uint64_t)) != 0) ERROR("Invalid block size: " << block_size_bytes);
    m_ptr_buffer.reset(new uint64_t[3 * m_max_num_edges]);
}

EdgeBlockLoader::~EdgeBlockLoader(){
    /* nop */
}

EdgeBlockReader EdgeBlockLoader::load(){
    uint64_t num_edges = m_loader.load(m_ptr_buffer.get(), m_max_num_edges);
    return EdgeBlockReader{m_ptr_buffer, num_edges};
}

/*
    Log Reader
*/
LogReader::LogReader(const string& path): m_loader(m_handle, stoull(parse_properties(path)["internal.edges.block_size"])) {
    auto properties = parse_properties(path);
    // print_properties(properties);
    m_handle.open(path, ios_base::in | ios_base::binary);
    set_marker(properties, m_handle, Section::EDGES);
    m_reader = m_loader.load();
}

LogReader::~LogReader() {
    m_handle.close();
}

bool LogReader::read_edge(uint64_t& src, uint64_t& des, double& weight) {
    if(!m_reader.has_next()){ // retrieve the next block of edges
        m_reader = m_loader.load();
    }

    return m_reader.read_edge(src, des, weight);
}

unordered_map<string, string> LogReader::parse_properties(const string& path) {
    fstream handle{path, ios_base::in | ios_base::binary};
    if(!handle.good()) LOG("Cannot open the file: " << path);
    
    unordered_map<string, string> properties;
    string line;

    getline(handle, line);
    if(line != "# GRAPHLOG") LOG("Missing magic header '# GRAPHLOG'");

    regex pattern { "^\\s*([A-Za-z0-9_.-]+?)\\s*=\\s*([^#\n]+?)\\s*" };
    while(!handle.eof()){
        string line;
        getline(handle, line);
        if(line == "__BINARY_SECTION_FOLLOWS") break; // done

        smatch matches;
        if ( regex_match(line, matches, pattern) ) { // side effect => populate matches
            string key = matches[1];
            string value = matches[2];

            properties[key] = value;
        }
    }
    handle.close();

    return properties;
}

void LogReader::print_properties(unordered_map<string, string> properties) {
    for(auto x: properties) {
        LOG(x.first << ": " << x.second);
    }
}