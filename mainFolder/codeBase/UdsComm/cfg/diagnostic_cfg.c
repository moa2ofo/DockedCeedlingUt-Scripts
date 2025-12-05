#include "diagnostic_cfg.h"
#include "diagnostic_cfg_priv.h"
#define NULL ((void *)0)


void checkCurrentNad(uint8 currentNad, Std_ReturnType *result)
{
    (void)currentNad;
    *result = E_OK;
}

/* Check if message data length is valid */
void checkMsgDataLength(uint16_t dataLength, Std_ReturnType *result)
{
    if (dataLength > 0u && dataLength <= 32u) {
        *result = E_OK;
    } else {
        *result = E_NOT_OK;
    }
}

Std_ReturnType RdbiVhitOverVoltageFaultDiag_(uint8*const  output_pu8,
    uint8*const  size_pu8, uint8* const errCode_pu8)
{
  (void)size_pu8;
  (void)errCode_pu8;
  output_pu8[0] = 0x01; /* Example data */
  return E_OK;
}

Std_ReturnType Subfunction_Request_Out_Of_Range(uint8*const  output_pu8,
    uint8*const  size_pu8, uint8* const errCode_pu8)
{
  (void)output_pu8;
  (void)size_pu8;
  if(NULL != errCode_pu8)
  {
    *errCode_pu8 = 0x12;
  }
  return E_NOT_OK;
}

Std_ReturnType getHandlersForReadDataById(uint8 l_did_u8, uint16 l_did_cu16,  uint8 *l_diagBufSize_u8, Std_ReturnType *l_didSupported_,  uint8 *l_diagBuf_pu8)
{
    (void)l_did_u8;
    diagHandler_t l_handler_ = &Subfunction_Request_Out_Of_Range;
    Std_ReturnType l_result_ = E_OK;
    uint8 l_errCode_u8 = 0;
    
    switch (l_did_cu16)
    {
    /* IS_OVERVOLT_FLAG */
    case 0xF308:
        *l_diagBufSize_u8 = DID_F308_SIZE;
        l_handler_ = &RdbiVhitOverVoltageFaultDiag_;
        break;

    default:
        *l_didSupported_  = E_NOT_OK;
        l_result_ = E_NOT_OK;
        l_errCode_u8 = kLinDiagNrcRequestOutOfRange;
        break;
    }
    
    return l_result_ = l_handler_(l_diagBuf_pu8, l_diagBufSize_u8, &l_errCode_u8);
}

