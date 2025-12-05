/**
 * @file VoltMonitoring.h
 * @brief Public interface of the voltage monitoring module.
 *
 * @details
 * This module provides a debounced voltage monitoring mechanism with
 * undervoltage and overvoltage detection based on configurable thresholds,
 * hysteresis, and activation/deactivation times.
 *
 * The module exposes:
 * - A state machine with three states: UNDERVOLTAGE, NORMAL, OVERVOLTAGE.
 * - An initialization function to reset internal context.
 * - A cyclic function to be called periodically with the elapsed time.
 * - A getter to retrieve the current monitoring state.
 */

#ifndef VOLT_MONITORING_H
#define VOLT_MONITORING_H

#include <stdint.h>


/**
 * @enum VoltMon_State_t
 * @brief Voltage monitoring state machine states.
 *
 * @details
 * The state machine used by the voltage monitoring module can be in one of
 * the following states:
 * - #VOLT_MON_STATE_UNDERVOLTAGE: The measured voltage is considered below the
 *   configured undervoltage threshold (after debouncing).
 * - #VOLT_MON_STATE_NORMAL: The measured voltage is within the normal range,
 *   i.e. not in undervoltage or overvoltage conditions.
 * - #VOLT_MON_STATE_OVERVOLTAGE: The measured voltage is considered above the
 *   configured overvoltage threshold (after debouncing).
 */
typedef enum
{
    /** Voltage is below the undervoltage threshold (debounced condition). */
    VOLT_MON_STATE_UNDERVOLTAGE = 0,

    /** Voltage is within the acceptable range (no under/overvoltage). */
    VOLT_MON_STATE_NORMAL,

    /** Voltage is above the overvoltage threshold (debounced condition). */
    VOLT_MON_STATE_OVERVOLTAGE
} VoltMon_State_t;

/**
 * @brief Initialize the voltage monitoring module.
 *
 * @details
 * **Goal of the function**
 *
 * The purpose of this function is to bring the voltage monitoring module
 * into a known safe state before use. It:
 * - Sets the internal state machine to #VOLT_MON_STATE_NORMAL.
 * - Resets all internal timers used for activation and deactivation
 *   debouncing.
 *
 * This function shall be called once at system startup, before any call
 * to ::voltMonRun().
 *
 * @par Interface summary
 *
 * | Interface                                 | In | Out | Data type | Param | Data factor | Data offset | Data size | Data range | Data unit |
 * |-------------------------------------------|:--:|:---:|-----------|-------|------------:|------------:|----------:|-----------:|-----------|
 * | VoltMon_Ctx.state                         |    |  X  | enum      |   -   |      1      |           0 |         1 | {0,1,2}    | [-]       |
 * | VoltMon_Ctx.uvActivationTimer_ms          |    |  X  | uint16    |   -   |      1      |           0 |         1 | [0, 65535] | [ms]      |
 * | VoltMon_Ctx.ovActivationTimer_ms          |    |  X  | uint16    |   -   |      1      |           0 |         1 | [0, 65535] | [ms]      |
 * | VoltMon_Ctx.deactivationTimer_ms          |    |  X  | uint16    |   -   |      1      |           0 |         1 | [0, 65535] | [ms]      |
 *
 * @pre None.
 * @post The internal state is set to #VOLT_MON_STATE_NORMAL and all timers
 *       are cleared.
 *
 * @return None.
 */
void VoltMon_Init(void);

/**
 * @brief Execute the voltage monitoring state machine.
 *
 * @details
 * **Goal of the function**
 *
 * The purpose of this function is to supervise the supply voltage by comparing
 * the measured value against configured undervoltage and overvoltage thresholds.
 * The detection is debounced using activation/deactivation timers and hysteresis.
 *
 * The monitoring logic:
 * - Detects undervoltage and overvoltage conditions when thresholds are exceeded
 *   for at least the configured activation time.
 * - Returns to NORMAL state only when voltage re-enters the safe region for the
 *   required deactivation time.
 * - Uses three operation states: NORMAL(0), UNDERVOLTAGE(1), OVERVOLTAGE(2).
 *
 * @par Interface summary
 *
 * | Interface                                 | In | Out | Data type | Param | Data factor | Data offset | Data size | Data range   | Data unit |
 * |-------------------------------------------|:--:|:---:|-----------|-------|------------:|------------:|----------:|--------------|-----------|
 * | dt_ms                                     | X  |     | uint16    |   -   |      1      |           0 |         1 | [0, 1000]    | [ms]      |
 * | VoltMon_GetVoltageFct()                   | X  |     | uint16    |   -   |      1      |           0 |         1 | [0, 20000]   | [mV]      |
 * | VoltMon_GetUnderOn_mV()                   | X  |     | uint16    |   -   |      1      |           0 |         1 | [0, 20000]   | [mV]      |
 * | VoltMon_GetUnderOff_mV()                  | X  |     | uint16    |   -   |      1      |           0 |         1 | [0, 20000]   | [mV]      |
 * | VoltMon_GetOverOn_mV()                    | X  |     | uint16    |   -   |      1      |           0 |         1 | [0, 20000]   | [mV]      |
 * | VoltMon_GetOverOff_mV()                   | X  |     | uint16    |   -   |      1      |           0 |         1 | [0, 20000]   | [mV]      |
 * | VoltMon_ActivationTime_ms                 | X  |     | uint16    |   -   |      1      |           0 |         1 | [1, 5000]    | [ms]      |
 * | VoltMon_DeactivationTime_ms               | X  |     | uint16    |   -   |      1      |           0 |         1 | [1, 5000]    | [ms]      |
 * | VoltMon_Ctx.state                         | X  |  X  | enum      |   -   |      1      |           0 |         1 | {0,1,2}      | [-]       |
 * | VoltMon_Ctx.uvActivationTimer_ms          | X  |  X  | uint16    |   -   |      1      |           0 |         1 | [0, 65535]   | [ms]      |
 * | VoltMon_Ctx.ovActivationTimer_ms          | X  |  X  | uint16    |   -   |      1      |           0 |         1 | [0, 65535]   | [ms]      |
 * | VoltMon_Ctx.deactivationTimer_ms          | X  |  X  | uint16    |   -   |      1      |           0 |         1 | [0, 65535]   | [ms]      |
 *
 * @par Activity diagram (PlantUML)
 *
 * @startuml
 * start
 * :Read voltage_mV;
 * :Read thresholds: underOn, underOff, overOn, overOff;
 *
 * if (state == NORMAL) then (NORMAL)
 *   :Reset deactivationTimer;
 *   if (voltage_mV <= underOn) then (UV ON)
 *       :uvActivationTimer += dt_ms;\novActivationTimer = 0;
 *       if (uvActivationTimer >= ActivationTime) then (UV TRIG)
 *           :state = UNDERVOLTAGE;\nuvActivationTimer = 0;
 *       endif
 *   else if (voltage_mV >= overOn) then (OV ON)
 *       :ovActivationTimer += dt_ms;\nuvActivationTimer = 0;
 *       if (ovActivationTimer >= ActivationTime) then (OV TRIG)
 *           :state = OVERVOLTAGE;\novActivationTimer = 0;
 *       endif
 *   else (NORMAL BAND)
 *       :Reset uvActivationTimer and ovActivationTimer;
 *   endif
 *
 * else if (state == UNDERVOLTAGE) then (UV)
 *   :Reset activation timers;
 *   if (voltage_mV >= underOff) then (RECOVER BAND UV)
 *       :deactivationTimer += dt_ms;
 *       if (deactivationTimer >= DeactivationTime) then (RECOVER UV)
 *           :state = NORMAL;\ndeactivationTimer = 0;
 *       endif
 *   else (STILL UV)
 *       :Reset deactivationTimer;
 *   endif
 *
 * else if (state == OVERVOLTAGE) then (OV)
 *   :Reset activation timers;
 *   if (voltage_mV <= overOff) then (RECOVER BAND OV)
 *       :deactivationTimer += dt_ms;
 *       if (deactivationTimer >= DeactivationTime) then (RECOVER OV)
 *           :state = NORMAL;\ndeactivationTimer = 0;
 *       endif
 *   else (STILL OV)
 *       :Reset deactivationTimer;
 *   endif
 *
 * else (INVALID)
 *   :Reset state and all timers;
 *   :state = NORMAL;
 * endif
 *
 * stop
 * @enduml
 *
 * @param dt_ms Elapsed time since the last call, in milliseconds.
 *
 * @return None.  
 * The function updates the internal state and timers of the Voltage Monitoring module.
 */
void voltMonRun(uint16_t dt_ms);

/**
 * @brief Get the current voltage monitoring state.
 *
 * @details
 * This function returns the current state of the internal voltage
 * monitoring state machine. It can be used by other modules to:
 * - React to undervoltage or overvoltage conditions.
 * - Implement higher-level fault handling or derating strategies.
 *
 * The returned value is a snapshot of the state at the time of the call.
 * The state is updated only by ::voltMonRun().
 *
 * @par Interface summary
 *
 * | Interface         | In | Out | Data type       | Param | Data factor | Data offset | Data size | Data range | Data unit |
 * |-------------------|:--:|:---:|-----------------|-------|------------:|------------:|----------:|-----------:|-----------|
 * | VoltMon_Ctx.state | X  |     | VoltMon_State_t |   -   |      1      |           0 |         1 | {0,1,2}    | [-]       |
 *
 * @return The current voltage monitoring state, see ::VoltMon_State_t.
 */
VoltMon_State_t VoltMon_GetState(void);

#endif /* VOLT_MONITORING_H */
