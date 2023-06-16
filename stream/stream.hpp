#ifndef STREAM
#define STREAM

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
    vector<uint64_t> generate_permutation_vector(uint64_t size);
    void permute(vector<uint64_t>& array1, vector<uint64_t>& array2, const vector<uint64_t>& permutation);
};

#endif