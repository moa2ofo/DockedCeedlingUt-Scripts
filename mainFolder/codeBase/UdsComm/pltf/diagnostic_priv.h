#include "diagnostic.h"
#include "diagnostic_cfg.h"
#include <stddef.h>


/* Send positive response */
void LinDiagSendPosResponse(void);

/* Send negative response with error code */
void LinDiagSendNegResponse(uint8_t errorCode);