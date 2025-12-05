

#ifndef DIAGNOSTIC_CFG_PRIV_H
#define DIAGNOSTIC_CFG_PRIV_H

#include "diagnostic_cfg.h"

#define DID_F308_SIZE 1U

typedef Std_ReturnType (*diagHandler_t)(uint8*const  output_pu8, uint8*const  size_pu8,
                                        uint8* const errCode_pu8);

Std_ReturnType RdbiVhitOverVoltageFaultDiag_(uint8*const  output_pu8,
    uint8*const  size_pu8, uint8* const errCode_pu8);

Std_ReturnType Subfunction_Request_Out_Of_Range(uint8*const  output_pu8,
    uint8*const  size_pu8, uint8* const errCode_pu8);

#endif