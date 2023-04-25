#include <fstream>
#include <iostream>
#include <regex>

#include "reader.hpp"

using namespace std;

static bool ignore_line(const string& line) {
    auto line_c = line.c_str();
    if(line_c == nullptr) return true;

    while(isspace(line_c[0])) line_c++;

    return line_c[0] == '#' || line_c[0] == '\0';
}

static bool is_number(const char* marker) {
    return marker != nullptr && (marker[0] >= '0' && marker[0] <= '9');
}

static string get_containing_folder(const string& path) {
    int k = path.length() - 1;

    while(k > 0) {
        if(path[k] == '/') break;
        k--;
    }
    k++;

    string ans = "";
    for(int i = 0; i < k; i++) ans += path[i];

    return ans;
}

Reader::Reader(const string& path) {
    parse_properties(path);
    // cout << vertex_file << " " << edge_file << endl;
    handle_vertex_file.open(vertex_file, ios::in);
    handle_edge_file.open(edge_file, ios::in);
}

Reader::~Reader() {
    handle_vertex_file.close();
    handle_edge_file.close();
}

void Reader::parse_properties(const string& path) {
    // cout << "path: " << get_containing_folder(path) << endl;
    fstream handle { path.c_str() };
    regex vertex_file_regex {"^\\s*graph.graph500-22.vertex-file\\s*=\\s*([^#\n]+?)\\s*"};
    regex edge_file_regex {"^\\s*graph.graph500-22.edge-file\\s*=\\s*([^#\n]+?)\\s*"};
    regex n_vertices_regex {"^\\s*graph.graph500-22.meta.vertices\\s*=\\s*([^#\n]+?)\\s*"};
    regex n_edges_regex {"^\\s*graph.graph500-22.meta.edges\\s*=\\s*([^#\n]+?)\\s*"};

    string parent = get_containing_folder(path);
    while(handle.good()) {
        string line;
        getline(handle, line);
        smatch matches;
        
        if(regex_match(line, matches, vertex_file_regex)) {
            vertex_file = parent + (string)matches[1];
        }

        if(regex_match(line, matches, edge_file_regex)) {
            edge_file = parent + (string)matches[1];
        }

        if(regex_match(line, matches, n_vertices_regex)) {
            n_vertices = stoull(matches[1]);
        }

        if(regex_match(line, matches, n_edges_regex)) {
            n_edges = stoull(matches[1]);
        }
    }
}

bool Reader::read_edge(uint64_t& src, uint64_t& dst) {
    if(!handle_edge_file.good()) {
        // cout << "Not good" << endl;
        return false;
    }
    
    // Reading line which is non-empty and non-comment
    string cur_line;
    bool skip = true;
    do {
        getline(handle_edge_file, cur_line);
        skip = ignore_line(cur_line);
    } while(skip && handle_edge_file.good());
    if(skip) return false;

    char* next { nullptr };
    const char* cur = cur_line.c_str();
    while(isspace(cur[0])) cur++;
    // if(!is_number(cur)) ERROR("line: `" + cur_line + "', cannot read the source vertex");
    src = strtoull(cur, &next, /* base */ 10);

    while(isspace(next[0])) next++;
    cur = next;
    // if(!is_number(cur)) ERROR("line: `" + cur_line + "', cannot read the destination vertex");
    dst = strtoull(cur, &next, 10);

    return true;
}

bool Reader::read_vertex(uint64_t& vtx) {
    if(!handle_vertex_file.good()) {
        // cout << "Not good" << endl;
        return false;
    }
    
    // Reading line which is non-empty and non-comment
    string cur_line;
    bool skip = true;
    do {
        getline(handle_vertex_file, cur_line);
        skip = ignore_line(cur_line);
    } while(skip && handle_vertex_file.good());
    if(skip) return false;

    const char* cur = cur_line.c_str();
    while(isspace(cur[0])) cur++;

    vtx = strtoull(cur, nullptr, 10);

    return true;
}