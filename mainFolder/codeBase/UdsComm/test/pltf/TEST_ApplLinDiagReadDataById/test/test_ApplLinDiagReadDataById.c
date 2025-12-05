#include "unity.h"
#include "ApplLinDiagReadDataById.h"
#include "diagnostic_cfg.h"
#include "mock_diagnostic_cfg.h"

/* Global buffers normally provided by LIN stack */
uint8_t pbLinDiagBuffer[32];
/* Message length */
uint16_t g_linDiagDataLength = 0;

/* Mock functions */
void LinDiagSendPosResponse(void);
void LinDiagSendNegResponse(uint8_t errorCode);

/* Test setup and teardown */
void setUp(void)
{
  /* Initialize buffers before each test */
  memset(pbLinDiagBuffer, 0, sizeof(pbLinDiagBuffer));
  g_linDiagDataLength = 0;
}

void tearDown(void)
{
}

/* ============================================================================
 * Test Cases: Success Path Tests
 * ============================================================================
 */

/**
 * Test: ApplLinDiagReadDataById_SuccessfulReadWithValidData
 * Description: Test successful read data by ID operation with valid NAD and message length
 * Expected: Positive response is sent, data length is updated with buffer size + 2
 */
void test_ApplLinDiagReadDataById_SuccessfulReadWithValidData(void)
{
  /* Setup */
  uint8_t test_data[4] = {0x22, 0xF3, 0x08, 0x00};
  memcpy(pbLinDiagBuffer, test_data, sizeof(test_data));
  g_linDiagDataLength = 3;
  
  uint8_t response_buffer[8] = {0xAA, 0xBB};
  uint8_t response_size = 2;

  /* Expectations */
  expect_checkCurrentNad(0, NULL);
  expect_checkCurrentNad_args_l_result_(E_OK);
  
  expect_checkMsgDataLength(3, NULL);
  expect_checkMsgDataLength_args_l_result_(E_OK);
  
  expect_getHandlersForReadDataById(0, 0xF308, NULL, NULL, response_buffer);
  expect_getHandlersForReadDataById_args_l_diagBufSize_(2);
  expect_getHandlersForReadDataById_args_l_didSupported_(E_OK);
  expect_getHandlersForReadDataById_ReturnThruPtr_l_diagBuf_pu8(response_buffer, 2);
  expect_getHandlersForReadDataById_and_return(E_OK);
  
  expect_LinDiagSendPosResponse();

  /* Execute */
  ApplLinDiagReadDataById();

  /* Verify */
  TEST_ASSERT_EQUAL_INT(4, g_linDiagDataLength); /* 2 + 2 */
}

/**
 * Test: ApplLinDiagReadDataById_DifferentValidDID
 * Description: Test with different valid DID value
 * Expected: Positive response is sent with correct response data
 */
void test_ApplLinDiagReadDataById_DifferentValidDID(void)
{
  /* Setup */
  uint8_t test_data[5] = {0x22, 0x12, 0x34, 0x00, 0x00};
  memcpy(pbLinDiagBuffer, test_data, sizeof(test_data));
  g_linDiagDataLength = 3;
  
  uint8_t response_buffer[8] = {0x11, 0x22, 0x33};
  uint8_t response_size = 3;

  /* Expectations */
  expect_checkCurrentNad(0, NULL);
  expect_checkCurrentNad_args_l_result_(E_OK);
  
  expect_checkMsgDataLength(3, NULL);
  expect_checkMsgDataLength_args_l_result_(E_OK);
  
  expect_getHandlersForReadDataById(0, 0x1234, NULL, NULL, response_buffer);
  expect_getHandlersForReadDataById_args_l_diagBufSize_(3);
  expect_getHandlersForReadDataById_args_l_didSupported_(E_OK);
  expect_getHandlersForReadDataById_ReturnThruPtr_l_diagBuf_pu8(response_buffer, 3);
  expect_getHandlersForReadDataById_and_return(E_OK);
  
  expect_LinDiagSendPosResponse();

  /* Execute */
  ApplLinDiagReadDataById();

  /* Verify */
  TEST_ASSERT_EQUAL_INT(5, g_linDiagDataLength); /* 3 + 2 */
}

/* ============================================================================
 * Test Cases: NAD Validation Failure Tests
 * ============================================================================
 */

/**
 * Test: ApplLinDiagReadDataById_InvalidNAD
 * Description: Test when current NAD check fails
 * Expected: Negative response is sent, further processing stops
 */
void test_ApplLinDiagReadDataById_InvalidNAD(void)
{
  /* Setup */
  uint8_t test_data[4] = {0x22, 0xF3, 0x08, 0x00};
  memcpy(pbLinDiagBuffer, test_data, sizeof(test_data));
  g_linDiagDataLength = 3;
  uint8_t error_code = 0x12;

  /* Expectations */
  expect_checkCurrentNad(0, NULL);
  expect_checkCurrentNad_args_l_result_(E_NOT_OK);
  
  expect_LinDiagSendNegResponse(error_code);

  /* Execute */
  ApplLinDiagReadDataById();

  /* Verify - data length should not be updated */
  TEST_ASSERT_EQUAL_INT(3, g_linDiagDataLength);
}

/* ============================================================================
 * Test Cases: Message Length Validation Failure Tests
 * ============================================================================
 */

/**
 * Test: ApplLinDiagReadDataById_InvalidMessageLength
 * Description: Test when message data length check fails
 * Expected: Negative response is sent, handler not called
 */
void test_ApplLinDiagReadDataById_InvalidMessageLength(void)
{
  /* Setup */
  uint8_t test_data[4] = {0x22, 0xF3, 0x08, 0x00};
  memcpy(pbLinDiagBuffer, test_data, sizeof(test_data));
  g_linDiagDataLength = 65535; /* Invalid length */
  uint8_t error_code = 0x31;

  /* Expectations */
  expect_checkCurrentNad(0, NULL);
  expect_checkCurrentNad_args_l_result_(E_OK);
  
  expect_checkMsgDataLength(65535, NULL);
  expect_checkMsgDataLength_args_l_result_(E_NOT_OK);
  
  expect_LinDiagSendNegResponse(error_code);

  /* Execute */
  ApplLinDiagReadDataById();

  /* Verify */
  TEST_ASSERT_EQUAL_INT(65535, g_linDiagDataLength);
}

/* ============================================================================
 * Test Cases: Handler/DID Processing Failure Tests
 * ============================================================================
 */

/**
 * Test: ApplLinDiagReadDataById_UnsupportedDID
 * Description: Test when requested DID is not supported
 * Expected: Negative response is sent with appropriate error code
 */
void test_ApplLinDiagReadDataById_UnsupportedDID(void)
{
  /* Setup */
  uint8_t test_data[4] = {0x22, 0xFF, 0xFF, 0x00};
  memcpy(pbLinDiagBuffer, test_data, sizeof(test_data));
  g_linDiagDataLength = 3;
  uint8_t error_code = 0x31; /* Request out of range */

  /* Expectations */
  expect_checkCurrentNad(0, NULL);
  expect_checkCurrentNad_args_l_result_(E_OK);
  
  expect_checkMsgDataLength(3, NULL);
  expect_checkMsgDataLength_args_l_result_(E_OK);
  
  expect_getHandlersForReadDataById(0, 0xFFFF, NULL, NULL, NULL);
  expect_getHandlersForReadDataById_args_l_didSupported_(E_NOT_OK);
  expect_getHandlersForReadDataById_and_return(E_NOT_OK);
  
  expect_LinDiagSendNegResponse(error_code);

  /* Execute */
  ApplLinDiagReadDataById();

  /* Verify - data length should not be updated */
  TEST_ASSERT_EQUAL_INT(3, g_linDiagDataLength);
}

/**
 * Test: ApplLinDiagReadDataById_HandlerError
 * Description: Test when handler returns an error during processing
 * Expected: Negative response is sent
 */
void test_ApplLinDiagReadDataById_HandlerError(void)
{
  /* Setup */
  uint8_t test_data[4] = {0x22, 0xF3, 0x08, 0x00};
  memcpy(pbLinDiagBuffer, test_data, sizeof(test_data));
  g_linDiagDataLength = 3;
  uint8_t error_code = 0x22;

  /* Expectations */
  expect_checkCurrentNad(0, NULL);
  expect_checkCurrentNad_args_l_result_(E_OK);
  
  expect_checkMsgDataLength(3, NULL);
  expect_checkMsgDataLength_args_l_result_(E_OK);
  
  expect_getHandlersForReadDataById(0, 0xF308, NULL, NULL, NULL);
  expect_getHandlersForReadDataById_args_l_didSupported_(E_NOT_OK);
  expect_getHandlersForReadDataById_and_return(E_NOT_OK);
  
  expect_LinDiagSendNegResponse(error_code);

  /* Execute */
  ApplLinDiagReadDataById();

  /* Verify */
  TEST_ASSERT_EQUAL_INT(3, g_linDiagDataLength);
}

/* ============================================================================
 * Test Cases: DID Extraction Tests
 * ============================================================================
 */

/**
 * Test: ApplLinDiagReadDataById_DIDExtractionFromBuffer
 * Description: Test correct extraction of DID from buffer bytes
 * Expected: DID is correctly extracted as 16-bit value from bytes 1 and 2
 */
void test_ApplLinDiagReadDataById_DIDExtractionFromBuffer(void)
{
  /* Setup - DID should be extracted as (byte1 << 8) | byte2 = 0xABCD */
  uint8_t test_data[4] = {0x22, 0xAB, 0xCD, 0x00};
  memcpy(pbLinDiagBuffer, test_data, sizeof(test_data));
  g_linDiagDataLength = 3;
  
  uint8_t response_buffer[2] = {0x12, 0x34};

  /* Expectations */
  expect_checkCurrentNad(0, NULL);
  expect_checkCurrentNad_args_l_result_(E_OK);
  
  expect_checkMsgDataLength(3, NULL);
  expect_checkMsgDataLength_args_l_result_(E_OK);
  
  /* Verify correct DID extraction: (0xAB << 8) | 0xCD = 0xABCD */
  expect_getHandlersForReadDataById(0, 0xABCD, NULL, NULL, response_buffer);
  expect_getHandlersForReadDataById_args_l_diagBufSize_(2);
  expect_getHandlersForReadDataById_args_l_didSupported_(E_OK);
  expect_getHandlersForReadDataById_ReturnThruPtr_l_diagBuf_pu8(response_buffer, 2);
  expect_getHandlersForReadDataById_and_return(E_OK);
  
  expect_LinDiagSendPosResponse();

  /* Execute */
  ApplLinDiagReadDataById();

  /* Verify */
  TEST_ASSERT_EQUAL_INT(4, g_linDiagDataLength);
}

/* ============================================================================
 * Test Cases: Edge Case Tests
 * ============================================================================
 */

/**
 * Test: ApplLinDiagReadDataById_MinimumDataLength
 * Description: Test with minimum valid data length
 * Expected: Response processed correctly
 */
void test_ApplLinDiagReadDataById_MinimumDataLength(void)
{
  /* Setup */
  uint8_t test_data[4] = {0x22, 0x00, 0x01, 0x00};
  memcpy(pbLinDiagBuffer, test_data, sizeof(test_data));
  g_linDiagDataLength = 3;
  
  uint8_t response_buffer[1] = {0xFF};

  /* Expectations */
  expect_checkCurrentNad(0, NULL);
  expect_checkCurrentNad_args_l_result_(E_OK);
  
  expect_checkMsgDataLength(3, NULL);
  expect_checkMsgDataLength_args_l_result_(E_OK);
  
  expect_getHandlersForReadDataById(0, 0x0001, NULL, NULL, response_buffer);
  expect_getHandlersForReadDataById_args_l_diagBufSize_(1);
  expect_getHandlersForReadDataById_args_l_didSupported_(E_OK);
  expect_getHandlersForReadDataById_ReturnThruPtr_l_diagBuf_pu8(response_buffer, 1);
  expect_getHandlersForReadDataById_and_return(E_OK);
  
  expect_LinDiagSendPosResponse();

  /* Execute */
  ApplLinDiagReadDataById();

  /* Verify */
  TEST_ASSERT_EQUAL_INT(3, g_linDiagDataLength); /* 1 + 2 */
}

/**
 * Test: ApplLinDiagReadDataById_LargeResponseData
 * Description: Test with large response data size
 * Expected: Response length correctly calculated
 */
void test_ApplLinDiagReadDataById_LargeResponseData(void)
{
  /* Setup */
  uint8_t test_data[4] = {0x22, 0xF3, 0x08, 0x00};
  memcpy(pbLinDiagBuffer, test_data, sizeof(test_data));
  g_linDiagDataLength = 3;
  
  uint8_t response_buffer[28] = {0};
  uint8_t i;
  for (i = 0; i < 28; i++) {
    response_buffer[i] = i;
  }

  /* Expectations */
  expect_checkCurrentNad(0, NULL);
  expect_checkCurrentNad_args_l_result_(E_OK);
  
  expect_checkMsgDataLength(3, NULL);
  expect_checkMsgDataLength_args_l_result_(E_OK);
  
  expect_getHandlersForReadDataById(0, 0xF308, NULL, NULL, response_buffer);
  expect_getHandlersForReadDataById_args_l_diagBufSize_(28);
  expect_getHandlersForReadDataById_args_l_didSupported_(E_OK);
  expect_getHandlersForReadDataById_ReturnThruPtr_l_diagBuf_pu8(response_buffer, 28);
  expect_getHandlersForReadDataById_and_return(E_OK);
  
  expect_LinDiagSendPosResponse();

  /* Execute */
  ApplLinDiagReadDataById();

  /* Verify */
  TEST_ASSERT_EQUAL_INT(30, g_linDiagDataLength); /* 28 + 2 */
}




