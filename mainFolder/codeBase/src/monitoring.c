#include "monitoring.h"
#include "dependency.h"   // external dependency we will mock

/* FUNCTION TO TEST */


int monitoring(void) {
    int val = dependency_get_value();
    return val * 2;
}
