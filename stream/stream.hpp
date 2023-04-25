#include <cstdint>
#include <vector>
#include <string>

using namespace std;

class EdgeStream {
    vector<uint64_t> sources;
    vector<uint64_t> destinations;
public:
    EdgeStream(const string& path);
    ~EdgeStream();

    vector<uint64_t> get_sources();
    vector<uint64_t> get_destinations();
};