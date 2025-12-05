#include "unity.h"
#include "voltMonRun.h"
#include "mock_VoltMonitoring_priv.h"
#include "mock_VoltMonitoring_cfg.h"

VoltMon_Context_t VoltMon_Ctx;

#define SCHEDULER_BASE_TIME 10u

#define ACTIVATION_TIMER_STEPS (VoltMon_ActivationTime_ms / SCHEDULER_BASE_TIME)
#define DEACTIVATION_TIMER_STEPS (VoltMon_DeactivationTime_ms / SCHEDULER_BASE_TIME)

#define SET_UNDER_VOLTAHE_TH_VAL_MV (VoltMon_ThresholdUnder_mV)
#define SET_OVER_VOLTAGE_TH_VAL_MV (VoltMon_ThresholdOver_mV)
#define RESET_UNDER_VOLTAHE_TH_VAL_MV (VoltMon_ThresholdUnder_mV+VoltMon_Hysteresis_mV)
#define RESET_OVER_VOLTAGE_TH_VAL_MV (VoltMon_ThresholdOver_mV-VoltMon_Hysteresis_mV)

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

    voltMonRun(SCHEDULER_BASE_TIME);

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.uvActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.ovActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


void test_voltMonRun_NormalState_VoltageToUnderVoltage(void)
{
    /* Test: voltage within normal band should keep state NORMAL */
    /* Arrange */
    setUp();


    /* Act */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_UNDER_VOLTAHE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.uvActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.ovActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


void test_voltMonRun_NormalState_VoltageToOverVoltage(void)
{
    /* Test: voltage within normal band should keep state NORMAL */
    /* Arrange */
    setUp();


    /* Act */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_OVER_VOLTAGE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_OVERVOLTAGE, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.uvActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.ovActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


/* ============================================================================
 * voltMonRun Tests - UNDERVOLTAGE State Transitions
 * ============================================================================ */

void test_voltMonRun_UnderVoltageState_VoltageRecoveryToNormal(void)
{
    /* Test: transition from UNDERVOLTAGE back to NORMAL state */
    /* Arrange */
    setUp();

    /* First, transition to UNDERVOLTAGE state */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_UNDER_VOLTAHE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);

    /* Act - Recover voltage above underOff threshold */
    for(int i = 0; i < DEACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(RESET_UNDER_VOLTAHE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


void test_voltMonRun_UnderVoltageState_VoltageStaysUnderVoltage(void)
{
    /* Test: voltage stays below underOff threshold - remains in UNDERVOLTAGE */
    /* Arrange */
    setUp();

    /* Transition to UNDERVOLTAGE state */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_UNDER_VOLTAHE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);

    /* Act - Stay below underOff threshold */
    for(int i = 0; i < 5; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_UNDER_VOLTAHE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


void test_voltMonRun_UnderVoltageState_PartialRecovery_ResetTimer(void)
{
    /* Test: voltage crosses underOff but drops back before deactivation timeout */
    /* Arrange */
    setUp();

    /* Transition to UNDERVOLTAGE state */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_UNDER_VOLTAHE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);

    /* Act - Voltage crosses above underOff for a bit, then drops back */
    for(int i = 0; i < DEACTIVATION_TIMER_STEPS-1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(RESET_UNDER_VOLTAHE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Drop back below underOff - timer should reset */
    VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_UNDER_VOLTAHE_TH_VAL_MV);
    VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
    VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
    VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
    VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

    voltMonRun(SCHEDULER_BASE_TIME);

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


/* ============================================================================
 * voltMonRun Tests - OVERVOLTAGE State Transitions
 * ============================================================================ */

void test_voltMonRun_OverVoltageState_VoltageRecoveryToNormal(void)
{
    /* Test: transition from OVERVOLTAGE back to NORMAL state */
    /* Arrange */
    setUp();

    /* First, transition to OVERVOLTAGE state */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_OVER_VOLTAGE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_OVERVOLTAGE, VoltMon_Ctx.state);

    /* Act - Recover voltage below overOff threshold */
    for(int i = 0; i < DEACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(RESET_OVER_VOLTAGE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


/* ============================================================================
 * voltMonRun Tests - Normal State Debouncing (timer accumulation)
 * ============================================================================ */

void test_voltMonRun_NormalState_UnderVoltageDebouncing_IncompleteTimer(void)
{
    /* Test: voltage below underOn but not long enough to trigger transition */
    /* Arrange */
    setUp();

    /* Act - Apply under-voltage for less than activation time */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS-1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_UNDER_VOLTAHE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert - Still in NORMAL state since timer didn't reach threshold */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
}


void test_voltMonRun_NormalState_OverVoltageDebouncing_IncompleteTimer(void)
{
    /* Test: voltage above overOn but not long enough to trigger transition */
    /* Arrange */
    setUp();

    /* Act - Apply over-voltage for less than activation time */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS-1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_OVER_VOLTAGE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert - Still in NORMAL state since timer didn't reach threshold */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
}


void test_voltMonRun_NormalState_VoltageOscillation_NoTransition(void)
{
    /* Test: voltage oscillates around thresholds without triggering transition */
    /* Arrange */
    setUp();

    /* Act - Alternate between under and over voltage */
    for(int i = 0; i < 3; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_UNDER_VOLTAHE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);
        voltMonRun(SCHEDULER_BASE_TIME);

        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(10000u);  /* Normal */
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);
        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.uvActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.ovActivationTimer_ms);
}


/* ============================================================================
 * voltMonRun Tests - Hysteresis Behavior
 * ============================================================================ */

void test_voltMonRun_Hysteresis_UnderVoltage_OffThreshold(void)
{
    /* Test: verify correct use of underOff threshold (with hysteresis) */
    /* Arrange */
    setUp();

    /* Transition to UNDERVOLTAGE state */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_UNDER_VOLTAHE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);

    /* Act - Voltage at exactly underOff threshold */
    for(int i = 0; i < DEACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(8500u);  /* underOff */
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
}


void test_voltMonRun_Hysteresis_OverVoltage_OffThreshold(void)
{
    /* Test: verify correct use of overOff threshold (with hysteresis) */
    /* Arrange */
    setUp();

    /* Transition to OVERVOLTAGE state */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_OVER_VOLTAGE_TH_VAL_MV);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_OVERVOLTAGE, VoltMon_Ctx.state);

    /* Act - Voltage at exactly overOff threshold */
    for(int i = 0; i < DEACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(13000u);  /* overOff */
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
}


/* ============================================================================
 * voltMonRun Tests - Edge Cases and Combined Scenarios
 * ============================================================================ */

void test_voltMonRun_CyclicTransitions_NORMAL_UV_NORMAL(void)
{
    /* Test: complete cycle from NORMAL -> UNDERVOLTAGE -> NORMAL */
    /* Arrange */
    setUp();
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);

    /* Act - Step 1: Transition to UNDERVOLTAGE */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(7500u);  /* Below underOn */
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);

    /* Act - Step 2: Return to NORMAL */
    for(int i = 0; i < DEACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(9000u);  /* Above underOff */
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.uvActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.ovActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


void test_voltMonRun_CyclicTransitions_NORMAL_OV_NORMAL(void)
{
    /* Test: complete cycle from NORMAL -> OVERVOLTAGE -> NORMAL */
    /* Arrange */
    setUp();
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);

    /* Act - Step 1: Transition to OVERVOLTAGE */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(13500u);  /* Above overOn */
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_OVERVOLTAGE, VoltMon_Ctx.state);

    /* Act - Step 2: Return to NORMAL */
    for(int i = 0; i < DEACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(12000u);  /* Below overOff */
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.uvActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.ovActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


void test_voltMonRun_LongDurationUnderVoltage(void)
{
    /* Test: prolonged under-voltage condition */
    /* Arrange */
    setUp();

    /* Act - Transition to UNDERVOLTAGE and stay there for multiple cycles */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS+5; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(7000u);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


void test_voltMonRun_LongDurationOverVoltage(void)
{
    /* Test: prolonged over-voltage condition */
    /* Arrange */
    setUp();

    /* Act - Transition to OVERVOLTAGE and stay there for multiple cycles */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS+5; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(14000u);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_OVERVOLTAGE, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}


void test_voltMonRun_BoundaryVoltage_JustBelowUnderOn(void)
{
    /* Test: voltage just below underOn threshold in NORMAL state */
    /* Arrange */
    setUp();

    /* Act - Apply voltage just below underOn (8000) */
    for(int i = 0; i < 2; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(7999u);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert - Still transitioning, not yet UNDERVOLTAGE */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
}


void test_voltMonRun_BoundaryVoltage_JustAboveOverOn(void)
{
    /* Test: voltage just above overOn threshold in NORMAL state */
    /* Arrange */
    setUp();

    /* Act - Apply voltage just above overOn (12500) */
    for(int i = 0; i < 2; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(12501u);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert - Still transitioning, not yet OVERVOLTAGE */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
}


void test_voltMonRun_ZeroVoltage(void)
{
    /* Test: zero voltage (extreme under-voltage) */
    /* Arrange */
    setUp();

    /* Act - Apply zero voltage */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(0u);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);
}


void test_voltMonRun_VeryHighVoltage(void)
{
    /* Test: very high voltage (extreme over-voltage) */
    /* Arrange */
    setUp();

    /* Act - Apply very high voltage */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(65535u);  /* Max uint16 */
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_OVERVOLTAGE, VoltMon_Ctx.state);
}


void test_voltMonRun_TimerOverflow_LongElapsedTime(void)
{
    /* Test: large dt_ms values don't cause incorrect behavior */
    /* Arrange */
    setUp();

    /* Act - Single call with large dt that exceeds activation time */
    VoltMon_ReadVoltageProject_mV_ExpectAndReturn(SET_UNDER_VOLTAHE_TH_VAL_MV);
    VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
    VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
    VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
    VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);

    voltMonRun(1000u);  /* 1000ms at once */

    /* Assert - Should transition directly to UNDERVOLTAGE */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);
}


void test_voltMonRun_AlternatingStates_NORMAL_UV_OV_NORMAL(void)
{
    /* Test: transition through all three states sequentially */
    /* Arrange */
    setUp();

    /* Step 1: NORMAL -> UNDERVOLTAGE */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(7500u);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);
        voltMonRun(SCHEDULER_BASE_TIME);
    }
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_UNDERVOLTAGE, VoltMon_Ctx.state);

    /* Step 2: UNDERVOLTAGE -> NORMAL */
    for(int i = 0; i < DEACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(10000u);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);
        voltMonRun(SCHEDULER_BASE_TIME);
    }
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);

    /* Step 3: NORMAL -> OVERVOLTAGE */
    for(int i = 0; i < ACTIVATION_TIMER_STEPS; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(13500u);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);
        voltMonRun(SCHEDULER_BASE_TIME);
    }
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_OVERVOLTAGE, VoltMon_Ctx.state);

    /* Step 4: OVERVOLTAGE -> NORMAL */
    for(int i = 0; i < DEACTIVATION_TIMER_STEPS+1; i++)
    {
        VoltMon_ReadVoltageProject_mV_ExpectAndReturn(10000u);
        VoltMon_GetUnderOn_mV_ExpectAndReturn(8000u);
        VoltMon_GetUnderOff_mV_ExpectAndReturn(8500u);
        VoltMon_GetOverOn_mV_ExpectAndReturn(12500u);
        VoltMon_GetOverOff_mV_ExpectAndReturn(13000u);
        voltMonRun(SCHEDULER_BASE_TIME);
    }

    /* Assert - Final state should be NORMAL */
    TEST_ASSERT_EQUAL_INT(VOLT_MON_STATE_NORMAL, VoltMon_Ctx.state);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.uvActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.ovActivationTimer_ms);
    TEST_ASSERT_EQUAL_UINT16(0u, VoltMon_Ctx.deactivationTimer_ms);
}
