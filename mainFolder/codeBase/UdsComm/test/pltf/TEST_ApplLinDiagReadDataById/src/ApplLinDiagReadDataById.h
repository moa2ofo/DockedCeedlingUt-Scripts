

#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <stdint.h>

extern uint8_t pbLinDiagBuffer[32];
/* Message length */
extern uint16_t g_linDiagDataLength;

void ApplLinDiagReadDataById(void);

#endif