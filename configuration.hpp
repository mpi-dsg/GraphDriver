#ifndef CONFIGURATION
#define CONFIGURATION

#include <iostream>
#include <mutex>

using namespace std;

extern mutex _log_mutex;
#define LOG( msg ) { scoped_lock lock(_log_mutex); cout << msg << /* flush immediately */ endl; }


// #define ERROR ( msg ) { scoped_lock lock(_log_mutex); cout << <<"ERROR:" << msg << /* flush immediately */ endl; }

class Configuration {
public:
    Configuration();
    ~Configuration();

    void set_n_threads(int threads);
    int get_n_threads();

    void set_batch_size(uint64_t batch_size);
    uint64_t get_batch_size();

    void set_repetitions(int repetitions);
    int get_repetitions();

    void set_rate(uint64_t rate);
    uint64_t get_rate();

private:
    int n_threads;
    uint64_t batch_size;
    int repetitions;
    uint64_t rate;
};

Configuration& configuration();

#endif