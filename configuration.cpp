#include <string>

#include "configuration.hpp"

using namespace std;

mutex _log_mutex;

static Configuration g_configuration;
Configuration& configuration(){ return g_configuration; }

Configuration::Configuration() {}

Configuration::~Configuration() {}

void Configuration::set_n_threads(int argc, char* argv[]) {
    if(argc >=2) n_threads = stoi(argv[1]);
}

int Configuration::get_n_threads() {
    return n_threads;
}