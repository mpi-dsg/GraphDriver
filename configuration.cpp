#include <string>

#include "configuration.hpp"

using namespace std;

mutex _log_mutex;

static Configuration g_configuration;
Configuration& configuration(){ return g_configuration; }

Configuration::Configuration() {}

Configuration::~Configuration() {}

void Configuration::set_n_threads(int threads) {
    n_threads = threads;
}

int Configuration::get_n_threads() {
    return n_threads;
}