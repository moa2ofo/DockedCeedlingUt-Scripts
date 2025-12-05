#include "VoltMonitoring.h"
#include "VoltMonitoring_priv.h"
#include "VoltMonitoring_cfg.h"

VoltMon_Context_t VoltMon_Ctx;

/* Helper locali */
uint16_t VoltMon_GetUnderOn_mV(void)
{
    return VoltMon_ThresholdUnder_mV;
}

uint16_t VoltMon_GetUnderOff_mV(void)
{
    return (uint16_t)(VoltMon_ThresholdUnder_mV + VoltMon_Hysteresis_mV);
}

uint16_t VoltMon_GetOverOn_mV(void)
{
    return VoltMon_ThresholdOver_mV;
}

uint16_t VoltMon_GetOverOff_mV(void)
{
    /* Gestione semplice: OFF più basso di ON di Hyst */
    return (uint16_t)(VoltMon_ThresholdOver_mV - VoltMon_Hysteresis_mV);
}

void VoltMon_Init(void)
{
    VoltMon_Ctx.state = VOLT_MON_STATE_NORMAL;
    VoltMon_Ctx.uvActivationTimer_ms = 0u;
    VoltMon_Ctx.ovActivationTimer_ms = 0u;
    VoltMon_Ctx.deactivationTimer_ms = 0u;

}

void voltMonRun(uint16_t dt_ms)
{

    uint16_t voltage_mV = READ_VOLT_PROJECT_MV;

    uint16_t underOn_mV  = VoltMon_GetUnderOn_mV();
    uint16_t underOff_mV = VoltMon_GetUnderOff_mV();
    uint16_t overOn_mV   = VoltMon_GetOverOn_mV();
    uint16_t overOff_mV  = VoltMon_GetOverOff_mV();

    switch (VoltMon_Ctx.state)
    {
        case VOLT_MON_STATE_NORMAL:
        {
            /* Reset timer di disattivazione in stato normale */
            VoltMon_Ctx.deactivationTimer_ms = 0u;

            /* Controllo undervoltage */
            if (voltage_mV <= underOn_mV)
            {
                VoltMon_Ctx.uvActivationTimer_ms += dt_ms;
                VoltMon_Ctx.ovActivationTimer_ms = 0u;

                if (VoltMon_Ctx.uvActivationTimer_ms >= VoltMon_ActivationTime_ms)
                {
                    VoltMon_Ctx.state = VOLT_MON_STATE_UNDERVOLTAGE;
                    VoltMon_Ctx.uvActivationTimer_ms = 0u;
                }
            }
            /* Controllo overvoltage */
            else if (voltage_mV >= overOn_mV)
            {
                VoltMon_Ctx.ovActivationTimer_ms += dt_ms;
                VoltMon_Ctx.uvActivationTimer_ms = 0u;

                if (VoltMon_Ctx.ovActivationTimer_ms >= VoltMon_ActivationTime_ms)
                {
                    VoltMon_Ctx.state = VOLT_MON_STATE_OVERVOLTAGE;
                    VoltMon_Ctx.ovActivationTimer_ms = 0u;
                }
            }
            else
            {
                /* Dentro banda normale -> reset dei timer */
                VoltMon_Ctx.uvActivationTimer_ms = 0u;
                VoltMon_Ctx.ovActivationTimer_ms = 0u;
            }
        }
        break;

        case VOLT_MON_STATE_UNDERVOLTAGE:
        {
            VoltMon_Ctx.uvActivationTimer_ms = 0u;
            VoltMon_Ctx.ovActivationTimer_ms = 0u;

            /* Rientro in NORMAL solo se la tensione sale sopra la soglia di OFF
             * e resta lì per VoltMon_DeactivationTime_ms.
             */
            if (voltage_mV >= underOff_mV)
            {
                VoltMon_Ctx.deactivationTimer_ms += dt_ms;

                if (VoltMon_Ctx.deactivationTimer_ms >= VoltMon_DeactivationTime_ms)
                {
                    VoltMon_Ctx.state = VOLT_MON_STATE_NORMAL;
                    VoltMon_Ctx.deactivationTimer_ms = 0u;
                }
            }
            else
            {
                VoltMon_Ctx.deactivationTimer_ms = 0u;
            }
        }
        break;

        case VOLT_MON_STATE_OVERVOLTAGE:
        {
            VoltMon_Ctx.uvActivationTimer_ms = 0u;
            VoltMon_Ctx.ovActivationTimer_ms = 0u;

            /* Rientro in NORMAL solo se la tensione scende sotto la soglia di OFF
             * e resta lì per VoltMon_DeactivationTime_ms.
             */
            if (voltage_mV <= overOff_mV)
            {
                VoltMon_Ctx.deactivationTimer_ms += dt_ms;

                if (VoltMon_Ctx.deactivationTimer_ms >= VoltMon_DeactivationTime_ms)
                {
                    VoltMon_Ctx.state = VOLT_MON_STATE_NORMAL;
                    VoltMon_Ctx.deactivationTimer_ms = 0u;
                }
            }
            else
            {
                VoltMon_Ctx.deactivationTimer_ms = 0u;
            }
        }
        break;

        default:
        {
            /* Stato non valido -> reset */
            VoltMon_Ctx.state = VOLT_MON_STATE_NORMAL;
            VoltMon_Ctx.uvActivationTimer_ms = 0u;
            VoltMon_Ctx.ovActivationTimer_ms = 0u;
            VoltMon_Ctx.deactivationTimer_ms = 0u;
        }
        break;
    }
}

VoltMon_State_t VoltMon_GetState(void)
{
    return VoltMon_Ctx.state;
}


int main()
{
    return 0;
}
