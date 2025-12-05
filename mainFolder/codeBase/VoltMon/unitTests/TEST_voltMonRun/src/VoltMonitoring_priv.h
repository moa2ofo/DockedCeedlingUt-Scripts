#ifndef VOLT_MONITORING_PRIV_H
#define VOLT_MONITORING_PRIV_H

#include <stdint.h>
#include "voltMonRun.h"

/* Contesto interno del monitor (non esposto fuori dal modulo) */
typedef struct
{
    VoltMon_State_t state;

    /* Timer per attivazione (ms) */
    uint16_t uvActivationTimer_ms;
    uint16_t ovActivationTimer_ms;

    /* Timer per disattivazione (ms) */
    uint16_t deactivationTimer_ms;

} VoltMon_Context_t;



uint16_t VoltMon_GetUnderOn_mV(void);

uint16_t VoltMon_GetUnderOff_mV(void);

uint16_t VoltMon_GetOverOn_mV(void);

uint16_t VoltMon_GetOverOff_mV(void);

/* Contesto globale interno (definito in VoltMonitoring.c) */
extern VoltMon_Context_t VoltMon_Ctx;

#endif /* VOLT_MONITORING_PRIV_H */
