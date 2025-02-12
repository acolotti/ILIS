#include "debug.h"

using namespace std;

debug::debug(bool active){
    this->active = active;
}

void debug::setState(bool active){
    this->active = active;
}
