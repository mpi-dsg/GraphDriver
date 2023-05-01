#include "update_stream.hpp"
#include "../reader/log_reader.hpp"
#include "../configuration.hpp"

EdgeUpdate::EdgeUpdate(uint64_t src, uint64_t dst, bool insert) : src(src), dst(dst), insert(insert) {}

UpdateStream::UpdateStream(const string& path) {
    LOG("Using log path: " << path);
    auto reader = new LogReader(path);
    
    uint64_t src = -1, dst = -1;
    double wt = -1;
    while(reader->read_edge(src, dst, wt)) {
        auto update = new EdgeUpdate(src, dst, wt==0.0);
        updates.push_back(update);
    }
}

vector<EdgeUpdate*> UpdateStream::get_updates() {
    return updates;
}