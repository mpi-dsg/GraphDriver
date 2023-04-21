#include <iostream>

#include "livegraph_driver.hpp"

using namespace std;

int main(int argc, char* argv[]){
    auto driver = new LiveGraphDriver();
    auto graph = driver->get_graph();
    auto tx = graph->begin_transaction();

    cout << 12 << endl;
}