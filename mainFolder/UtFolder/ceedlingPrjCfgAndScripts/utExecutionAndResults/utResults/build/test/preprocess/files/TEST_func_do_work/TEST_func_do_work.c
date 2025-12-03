// CEEDLING NOTICE: This generated file only to be consumed for test runner creation

#include "utExecutionAndResults/utResults/build/vendor/unity/src/unity.h"
#include "utExecutionAndResults/utUnderTest/src/func_do_work.h"
#include "Mockdependency.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void testfunc_should_double_the_dependency_value(void)
{
    dependency_get_value_CMockExpectAndReturn(18, 5);

    int result = func_do_work();

    UnityAssertEqualNumber((UNITY_INT)((10)), (UNITY_INT)((result)), (
   ((void *)0)
   ), (UNITY_UINT)(24), UNITY_DISPLAY_STYLE_INT);
}

void testfunc_should_handle_zero(void)
{
    dependency_get_value_CMockExpectAndReturn(29, 0);

    int result = func_do_work();

    UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((result)), (
   ((void *)0)
   ), (UNITY_UINT)(33), UNITY_DISPLAY_STYLE_INT);
}