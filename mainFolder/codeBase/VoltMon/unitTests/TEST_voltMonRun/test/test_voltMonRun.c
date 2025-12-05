#include "unity.h"
#include "voltMonRun.h"
#include "mock_VoltMonitoring_priv.h"
#include "mock_VoltMonitoring_cfg.h"

VoltMon_Context_t VoltMon_Ctx;

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

void setUp(void)
{
    /* Reset context before each test */
    VoltMon_Ctx.state = VOLT_MON_STATE_NORMAL;
    VoltMon_Ctx.uvActivationTimer_ms = 0u;
    VoltMon_Ctx.ovActivationTimer_ms = 0u;
    VoltMon_Ctx.deactivationTimer_ms = 0u;
}

void tearDown(void)
{
}



/* ============================================================================
 * voltMonRun Tests - NORMAL State Transitions
 * ============================================================================ */

void test_voltMonRun_NormalState_VoltageWithinBand_RemainsNormal(void)
{
    /* Test: voltage within normal band should keep state NORMAL */
    /* Arrange */
    setUp();
    uint16_t voltage = 10000u;  /* between underOff(8500) and overOff(12500) */
    VoltMon_ReadVoltageProject_mV_ExpectAndReturn(voltage);

    /* Act */
    VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
    VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
    VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
    VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);
    voltMonRun(10u);

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.uvActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.ovActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}
