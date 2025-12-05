#ifndef VOLT_MONITORING_CFG_H
#define VOLT_MONITORING_CFG_H

#include <stdint.h>

/* Tipo funzione per leggere la tensione (in mV) */
#define READ_VOLT_PROJECT_MV VoltMon_ReadVoltageProject_mV()

/* Parametri di configurazione (tutti in cfg) */
extern const uint16_t VoltMon_ThresholdUnder_mV;      /* es. 8000 mV  */
extern const uint16_t VoltMon_ThresholdOver_mV;       /* es. 13000 mV */
extern const uint16_t VoltMon_Hysteresis_mV;          /* es. 500 mV   */

extern const uint16_t VoltMon_ActivationTime_ms;      /* es. 500 ms */
extern const uint16_t VoltMon_DeactivationTime_ms;    /* es. 500 ms */

/*
 * Periodo di chiamata di voltMonRun in ms.
 * Serve per convertire ms -> numero di cicli, se preferisci puoi NON usarlo
 * e passare dt_ms a voltMonRun.
 */
extern const uint16_t VoltMon_TaskPeriod_ms;

/* Facoltativo: prototipo di una funzione specifica di questo progetto
 * che legge la tensione e viene usata come target di VoltMon_GetVoltageFct.
 */
uint16_t VoltMon_ReadVoltageProject_mV(void);

#endif /* VOLT_MONITORING_CFG_H */
