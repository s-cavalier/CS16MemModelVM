#include "Manager.h"


kernel::SharedResources::SharedResources() : freeList(), memory(), processes() {}

kernel::SharedResources kernel::SharedResources::manager;