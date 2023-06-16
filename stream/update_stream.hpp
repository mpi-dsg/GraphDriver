#ifndef UPDATE_STREAM
#define UPDATE_STREAM

#include <cstdint>
#include <vector>
#include <string>

using namespace std;

class EdgeUpdate {
public:
    EdgeUpdate();
    EdgeUpdate(uint64_t src, uint64_t dst, bool insert);
    ~EdgeUpdate();


    uint64_t src, dst;
    bool insert;
};

class UpdateStream {
    vector<EdgeUpdate> updates;
    uint64_t size = 0;
public:
    UpdateStream();
    UpdateStream(const string& path);
    ~UpdateStream();

    void add_update(EdgeUpdate update);
    vector<EdgeUpdate>& get_updates();
    uint64_t get_size();
};

#endif