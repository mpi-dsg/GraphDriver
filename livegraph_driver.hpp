#include "livegraph/livegraph.hpp"

using namespace lg;
using namespace std;

class LiveGraphDriver{
public:
    LiveGraphDriver();
    ~LiveGraphDriver();

    Graph* get_graph();
    void load_graph(string path);

private:
    Graph* graph;
};