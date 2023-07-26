#ifndef SECUREMIRROR_RSA_H
#define SECUREMIRROR_RSA_H


/////  FILE INCLUDES  /////
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/encoder.h>
#include <openssl/decoder.h>

#include <openssl/err.h>




/////  DEFINITIONS  /////
#define KEY_BYTES_SIZE (512)
#define PADDING_BYTES_SIZE (42)
#define TEST_TEXT_SIZE (KEY_BYTES_SIZE) - (PADDING_BYTES_SIZE)




/////  FUNCTION PROTOTYPES  /////
int generate_and_write_key(unsigned long public_exponent, int key_bitsize, const char* private_key_filename, const char* public_key_filename, RSA** rsa_key_to_return);




#endif //!SECUREMIRROR_RSA_H
