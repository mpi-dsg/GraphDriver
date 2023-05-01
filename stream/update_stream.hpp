#include <cstdint>
#include <vector>
#include <string>

using namespace std;

class EdgeUpdate {
public:
    EdgeUpdate(uint64_t src, uint64_t dst, bool insert);
    ~EdgeUpdate();


    uint64_t src, dst;
    bool insert;
};

class UpdateStream {
    vector<EdgeUpdate*> updates;
public:
    UpdateStream(const string& path);
    ~UpdateStream();

     vector<EdgeUpdate*> get_updates();
};