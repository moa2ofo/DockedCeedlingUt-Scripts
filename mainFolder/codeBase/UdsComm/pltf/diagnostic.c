#include "diagnostic_priv.h"

/* Global buffers normally provided by LIN stack */
uint8_t pbLinDiagBuffer[32];
/* Message length */
uint16_t g_linDiagDataLength = 0;


/* Send positive response */
void LinDiagSendPosResponse(void)
{
    /* Implementation stub: send positive response via LIN */
}

/* Send negative response with error code */
void LinDiagSendNegResponse(uint8_t errorCode)
{
    /* Implementation stub: send negative response via LIN */
    (void)errorCode;
}

void ApplLinDiagReadDataById(void)
{
  const uint16_t l_did_cu16 = ((uint16_t)(pbLinDiagBuffer[1] << 8) & 0xFF00) |
                              ((uint16_t)pbLinDiagBuffer[2] & 0x00FF);
  Std_ReturnType l_result_ = E_OK;
  uint8_t l_errCode_u8 = 0;
  uint8_t * const l_diagBuf_pu8 = &pbLinDiagBuffer[3];
  uint8_t l_diagBufSize_u8 = 0;
  Std_ReturnType l_didSupported_ = E_OK;

  checkCurrentNad((uint8_t)0u, &l_result_);

  if (E_OK == l_result_) {
    checkMsgDataLength(g_linDiagDataLength, &l_result_);
  }

  if (E_OK == l_result_) {
    l_result_ = getHandlersForReadDataById(l_errCode_u8, l_did_cu16, &l_diagBufSize_u8, &l_didSupported_, l_diagBuf_pu8);
  }

  switch (l_result_)
  {
    case E_OK:
      g_linDiagDataLength = l_diagBufSize_u8 + 2u;
      LinDiagSendPosResponse();
      break;
    default:
      LinDiagSendNegResponse(l_errCode_u8);
      break;
  }
}


int main(void)
{
    return 0;
}
