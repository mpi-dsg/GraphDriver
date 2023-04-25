#include <iostream>
#include <mutex>

using namespace std;

extern mutex _log_mutex;
#define LOG( msg ) { scoped_lock lock(_log_mutex); cout << msg << /* flush immediately */ endl; }

class Configuration {
public:
    Configuration();
    ~Configuration();

    void set_n_threads(int argc, char* argv[]);
    int get_n_threads();

private:
    int n_threads;
};

Configuration& configuration();