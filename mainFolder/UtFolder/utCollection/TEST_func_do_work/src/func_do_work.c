#include "func_do_work.h"
#include "dependency.h"   // external dependency we will mock

/* FUNCTION TO TEST */


int func_do_work(void) {
    int val = dependency_get_value();
    return val * 2;
}
