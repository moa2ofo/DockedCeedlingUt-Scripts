#include "VoltMonitoring_cfg.h"

/* ---- VALORI DI CONFIGURAZIONE (progetto-dipendenti) ---- */

const uint16_t VoltMon_ThresholdUnder_mV   = 8000u;
const uint16_t VoltMon_ThresholdOver_mV    = 13000u;
const uint16_t VoltMon_Hysteresis_mV       = 500u;

const uint16_t VoltMon_ActivationTime_ms   = 500u;
const uint16_t VoltMon_DeactivationTime_ms = 500u;

/* Periodo task di monitoraggio (esempio: 10 ms) */
const uint16_t VoltMon_TaskPeriod_ms       = 10u;


/* Implementazione di esempio: qui metterai la vera lettura ADC / HAL */
uint16_t VoltMon_ReadVoltageProject_mV(void)
{
    /* TODO: sostituisci con lettura reale della tensione in mV */
    uint16_t dummyVoltage = 12000u;
    return dummyVoltage;
}
