#include "stdafx.h"

#include <time.h>
#include <iostream>
#include <stdlib.h>
//#include <omp.h>

#ifdef __linux__
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "memwatch.h"
#include <sys/time.h>
#include "LinuxPort.h"
#endif

#include "IBEncryption.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//const unsigned char AES_KEY_ENCRYPTION[32] =	{'0','1','2','3','4','5','6','7','8','9',
//												 '0','1','2','3','4','5','6','7','8','9',
//												 '0','1','2','3','4','5','6','7','8','9',
//												 '0','1'};
const unsigned char AES_KEY_ENCRYPTION[256] =	{0x30,	0x31,	0x32,	0x33,	0x34,	0x35,	0x36,	0x37,	0x38,	0x39,	0x30,	0x31,	0x32,	0x33,	0x34,	0x35,
												0x36,	0x37,	0x38,	0x39,	0x30,	0x31,	0x32,	0x33,	0x34,	0x35,	0x36,	0x37,	0x38,	0x39,	0x30,	0x31,
												0x23,	0x35,	0xf5,	0x34,	0x17,	0x00,	0xc3,	0x03,	0x2f,	0x39,	0xf3,	0x32,	0x1d,	0x0a,	0xc7,	0x07,
												0x92,	0x50,	0xfe,	0xfc,	0xa2,	0x61,	0xcc,	0xcf,	0x96,	0x54,	0xfa,	0xf8,	0xae,	0x6d,	0xca,	0xc9,
												0x1d,	0x41,	0x28,	0xd0,	0x0a,	0x41,	0xeb,	0xd3,	0x25,	0x78,	0x18,	0xe1,	0x38,	0x72,	0xdf,	0xe6,
												0x95,	0x10,	0x60,	0x72,	0x37,	0x71,	0xac,	0xbd,	0xa1,	0x25,	0x56,	0x45,	0x0f,	0x48,	0x9c,	0x8c,
												0x4b,	0x9f,	0x4c,	0xa6,	0x41,	0xde,	0xa7,	0x75,	0x64,	0xa6,	0xbf,	0x94,	0x5c,	0xd4,	0x60,	0x72,
												0xdf,	0x58,	0xb0,	0x32,	0xe8,	0x29,	0x1c,	0x8f,	0x49,	0x0c,	0x4a,	0xca,	0x46,	0x44,	0xd6,	0x46,
												0x58,	0x69,	0x16,	0xfc,	0x19,	0xb7,	0xb1,	0x89,	0x7d,	0x11,	0x0e,	0x1d,	0x21,	0xc5,	0x6e,	0x6f,
												0x22,	0xfe,	0x2f,	0x9a,	0xca,	0xd7,	0x33,	0x15,	0x83,	0xdb,	0x79,	0xdf,	0xc5,	0x9f,	0xaf,	0x99,
												0x93,	0x10,	0xf8,	0x5a,	0x8a,	0xa7,	0x49,	0xd3,	0xf7,	0xb6,	0x47,	0xce,	0xd6,	0x73,	0x29,	0xa1,
												0xd4,	0x71,	0x8a,	0xa8,	0x1e,	0xa6,	0xb9,	0xbd,	0x9d,	0x7d,	0xc0,	0x62,	0x58,	0xe2,	0x6f,	0xfb,
												0x2b,	0xb8,	0xf7,	0x30,	0xa1,	0x1f,	0xbe,	0xe3,	0x56,	0xa9,	0xf9,	0x2d,	0x80,	0xda,	0xd0,	0x8c,
												0x19,	0x26,	0xfa,	0xcc,	0x07,	0x80,	0x43,	0x71,	0x9a,	0xfd,	0x83,	0x13,	0xc2,	0x1f,	0xec,	0xe8,
												0xab,	0x76,	0x6c,	0x15,	0x0a,	0x69,	0xd2,	0xf6,	0x5c,	0xc0,	0x2b,	0xdb,	0xdc,	0x1a,	0xfb,	0x57,
												0x9f,	0x84,	0xf5,	0x97,	0x98,	0x04,	0xb6,	0xe6,	0x02,	0xf9,	0x35,	0xf5,	0xc0,	0xe6,	0xd9,	0x1d};

#define FE(x)  (((x) << 1) ^ ((((x)>>7) & 1) * 0x1b))
#define FD(x)  (((x) >> 1) ^ (((x) & 1) ? 0x8d : 0))

const unsigned char sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};
const unsigned char sboxinv[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,
    0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87,
    0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d,
    0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2,
    0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,
    0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,
    0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a,
    0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02,
    0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea,
    0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,
    0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89,
    0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20,
    0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31,
    0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,
    0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0,
    0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26,
    0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};


const unsigned char rj_xtime[256] = {	0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12,
										0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26,
										0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a,
										0x3c, 0x3e, 0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e,
										0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e, 0x60, 0x62,
										0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76,
										0x78, 0x7a, 0x7c, 0x7e, 0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 
										0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e, 
										0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae, 0xb0, 0xb2, 
										0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe, 0xc0, 0xc2, 0xc4, 0xc6, 
										0xc8, 0xca, 0xcc, 0xce, 0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 
										0xdc, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee, 
										0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe, 0x1b, 0x19, 
										0x1f, 0x1d, 0x13, 0x11, 0x17, 0x15, 0x0b, 0x09, 0x0f, 0x0d, 
										0x03, 0x01, 0x07, 0x05, 0x3b, 0x39, 0x3f, 0x3d, 0x33, 0x31, 
										0x37, 0x35, 0x2b, 0x29, 0x2f, 0x2d, 0x23, 0x21, 0x27, 0x25, 
										0x5b, 0x59, 0x5f, 0x5d, 0x53, 0x51, 0x57, 0x55, 0x4b, 0x49, 
										0x4f, 0x4d, 0x43, 0x41, 0x47, 0x45, 0x7b, 0x79, 0x7f, 0x7d, 
										0x73, 0x71, 0x77, 0x75, 0x6b, 0x69, 0x6f, 0x6d, 0x63, 0x61, 
										0x67, 0x65, 0x9b, 0x99, 0x9f, 0x9d, 0x93, 0x91, 0x97, 0x95, 
										0x8b, 0x89, 0x8f, 0x8d, 0x83, 0x81, 0x87, 0x85, 0xbb, 0xb9, 
										0xbf, 0xbd, 0xb3, 0xb1, 0xb7, 0xb5, 0xab, 0xa9, 0xaf, 0xad, 
										0xa3, 0xa1, 0xa7, 0xa5, 0xdb, 0xd9, 0xdf, 0xdd, 0xd3, 0xd1, 
										0xd7, 0xd5, 0xcb, 0xc9, 0xcf, 0xcd, 0xc3, 0xc1, 0xc7, 0xc5, 
										0xfb, 0xf9, 0xff, 0xfd, 0xf3, 0xf1, 0xf7, 0xf5, 0xeb, 0xe9, 
										0xef, 0xed, 0xe3, 0xe1, 0xe7, 0xe5};

const unsigned char rj_xtime_rj_xtime[256] = {	0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24,
												0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c, 0x40, 0x44, 0x48, 0x4c,
												0x50, 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74,
												0x78, 0x7c, 0x80, 0x84, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c,
												0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc, 0xc0, 0xc4,
												0xc8, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4, 0xe8, 0xec,
												0xf0, 0xf4, 0xf8, 0xfc, 0x1b, 0x1f, 0x13, 0x17, 0x0b, 0x0f,
												0x03, 0x07, 0x3b, 0x3f, 0x33, 0x37, 0x2b, 0x2f, 0x23, 0x27,
												0x5b, 0x5f, 0x53, 0x57, 0x4b, 0x4f, 0x43, 0x47, 0x7b, 0x7f,
												0x73, 0x77, 0x6b, 0x6f, 0x63, 0x67, 0x9b, 0x9f, 0x93, 0x97,
												0x8b, 0x8f, 0x83, 0x87, 0xbb, 0xbf, 0xb3, 0xb7, 0xab, 0xaf,
												0xa3, 0xa7, 0xdb, 0xdf, 0xd3, 0xd7, 0xcb, 0xcf, 0xc3, 0xc7,
												0xfb, 0xff, 0xf3, 0xf7, 0xeb, 0xef, 0xe3, 0xe7, 0x36, 0x32,
												0x3e, 0x3a, 0x26, 0x22, 0x2e, 0x2a, 0x16, 0x12, 0x1e, 0x1a,
												0x06, 0x02, 0x0e, 0x0a, 0x76, 0x72, 0x7e, 0x7a, 0x66, 0x62,
												0x6e, 0x6a, 0x56, 0x52, 0x5e, 0x5a, 0x46, 0x42, 0x4e, 0x4a,
												0xb6, 0xb2, 0xbe, 0xba, 0xa6, 0xa2, 0xae, 0xaa, 0x96, 0x92,
												0x9e, 0x9a, 0x86, 0x82, 0x8e, 0x8a, 0xf6, 0xf2, 0xfe, 0xfa,
												0xe6, 0xe2, 0xee, 0xea, 0xd6, 0xd2, 0xde, 0xda, 0xc6, 0xc2,
												0xce, 0xca, 0x2d, 0x29, 0x25, 0x21, 0x3d, 0x39, 0x35, 0x31,
												0x0d, 0x09, 0x05, 0x01, 0x1d, 0x19, 0x15, 0x11, 0x6d, 0x69,
												0x65, 0x61, 0x7d, 0x79, 0x75, 0x71, 0x4d, 0x49, 0x45, 0x41,
												0x5d, 0x59, 0x55, 0x51, 0xad, 0xa9, 0xa5, 0xa1, 0xbd, 0xb9,
												0xb5, 0xb1, 0x8d, 0x89, 0x85, 0x81, 0x9d, 0x99, 0x95, 0x91,
												0xed, 0xe9, 0xe5, 0xe1, 0xfd, 0xf9, 0xf5, 0xf1, 0xcd, 0xc9,
												0xc5, 0xc1, 0xdd, 0xd9, 0xd5, 0xd1};

unsigned char rj_xtime_f(unsigned char x);

CIBEncryption::CIBEncryption(const unsigned char* key)
    : m_buffer_pos(0)
    , m_remainingLength(0)
    , m_decryptInitialized(false)
{
	//memset(m_key,		0, KEY_SIZE);
    memset(m_salt,		0, KEY_SIZE);
	memset(m_rkey,		0, KEY_SIZE);
	//memset(m_rkey_all,	0, 1024);
	//memset(m_rkey_dec,	0, KEY_ALL_ROUND_SIZE);

	m_remainingLength = 0;

	memcpy(m_key, key, KEY_SIZE);
	memcpy(m_primary_key, key, KEY_SIZE);
	
	memcpy(m_rkey_all,	key, 256);
	//memcpy(m_rkey_dec,	key, 256);


	m_P			= CreateDecimal((unsigned char*)"139", 3);
	m_Q			= CreateDecimal((unsigned char*)"239", 3);
	m_E			= CreateDecimal((unsigned char *)"6569", 4);
	m_D			= CreateDecimal((unsigned char *)"5", 1);
	m_PublicKey	= RSAGetPublicKey(&m_P, &m_Q);


#if 0
	FILE *fp;
	fp = fopen("d:\\round_key_all.txt", "w"); 
	for(int index = 1 ; index <= KEY_ALL_ROUND_SIZE ; index++)
	{
		if( (index-1) % 16 == 0)
			fprintf(fp, ",\n");
		else
			fprintf(fp, ",\t");

		fprintf(fp, "0x%02x", m_rkey_all[index-1]);
	}
	fclose(fp);
#endif

}

CIBEncryption::~CIBEncryption()
{
	DestroyBuffer(m_P.digit);
	DestroyBuffer(m_Q.digit);
	DestroyBuffer(m_E.digit);
	DestroyBuffer(m_D.digit);
	DestroyBuffer(m_PublicKey.digit);
}

void CIBEncryption::AES_Decrypt(const unsigned char *key, unsigned char *encrypted, unsigned int encrypted_length, unsigned char *plain)
{
//	clock_t start, end;
//	start = clock();


	//------------------------------------------------------------------------------------------
	//
	// Init
	//
	
	memcpy(m_key, key, KEY_SIZE);
	m_remainingLength = encrypted_length;

	m_buffer_pos = 0;
	m_decrypted_pos = 0;

/*	m_decryptInitialized = false;


	if( !m_decryptInitialized )//&& (m_buffer_pos == m_salt.size() + 1) )
	{
		int	padding = 0;

		// Get salt
		//for( i = 0; i < m_salt.size(); i++)
		//	m_salt[j] = m_buffer[i];

		// Get padding
		//padding = (m_buffer[i] & 0xFF);
		m_remainingLength -= padding + 1;

		// Start decyprting
		//m_buffer_pos = 0;

		m_decryptInitialized = true;
	}
*/
	//------------------------------------------------------------------------------------------
	//
	// Decryption
	//

	//#pragma omp parallel for
	register unsigned int i ;
	for(i = 0; i < encrypted_length; i += BLOCK_SIZE)
	{
		memcpy(m_buffer, encrypted+i, BLOCK_SIZE);
		//m_buffer_pos = BLOCK_SIZE;

		check_and_decrypt_buffer(plain);
	}

//	end = clock();
//	TRACE("Elapsed decryption time = %d(ms)\n", end - start);
}

void CIBEncryption::AES_Decrypt_opt(unsigned char *encrypted, unsigned int encrypted_length, unsigned char *plain)
{
	register int i ;

	unsigned char *in_buf, *out_buf;

	for(i = 0; i < (int)encrypted_length; i+=BLOCK_SIZE)
	{
		//decrypt_opt(&encrypted[i], &plain[i]);
		in_buf = &encrypted[i];
		out_buf = &plain[i];

		add_round_key(in_buf, 14);
		shift_rows_inv_sub_bytes_inv_add_round_key(in_buf, tmp_buf, 13);
		mix_columns_inv(tmp_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 12);
		mix_columns_inv(out_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 11);
		mix_columns_inv(tmp_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 10);
		mix_columns_inv(out_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 9);
		mix_columns_inv(tmp_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 8);
		mix_columns_inv(out_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 7);
		mix_columns_inv(tmp_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 6);
		mix_columns_inv(out_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 5);
		mix_columns_inv(tmp_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 4);
		mix_columns_inv(out_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 3);
		mix_columns_inv(tmp_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 2);
		mix_columns_inv(out_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 1);
		mix_columns_inv(tmp_buf);

		shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 0);
	}
}

void CIBEncryption::expand_enc_key(unsigned char *rc)
{
	register unsigned char i;

    m_rkey[0] = m_rkey[0] ^ sbox[m_rkey[29]] ^ (*rc);
    m_rkey[1] = m_rkey[1] ^ sbox[m_rkey[30]];
    m_rkey[2] = m_rkey[2] ^ sbox[m_rkey[31]];
    m_rkey[3] = m_rkey[3] ^ sbox[m_rkey[28]];
    *rc = FE(*rc);

    for(i = 4; i < 16; i += 4) {
        m_rkey[i] = m_rkey[i] ^ m_rkey[i-4];
        m_rkey[i+1] = m_rkey[i+1] ^ m_rkey[i-3];
        m_rkey[i+2] = m_rkey[i+2] ^ m_rkey[i-2];
        m_rkey[i+3] = m_rkey[i+3] ^ m_rkey[i-1];
    }
    m_rkey[16] = m_rkey[16] ^ sbox[m_rkey[12]];
    m_rkey[17] = m_rkey[17] ^ sbox[m_rkey[13]];
    m_rkey[18] = m_rkey[18] ^ sbox[m_rkey[14]];
    m_rkey[19] = m_rkey[19] ^ sbox[m_rkey[15]];

    for(i = 20; i < 32; i += 4) {
        m_rkey[i] = m_rkey[i] ^ m_rkey[i-4];
        m_rkey[i+1] = m_rkey[i+1] ^ m_rkey[i-3];
        m_rkey[i+2] = m_rkey[i+2] ^ m_rkey[i-2];
        m_rkey[i+3] = m_rkey[i+3] ^ m_rkey[i-1];
    }
}

void CIBEncryption::expand_dec_key(unsigned char* rc)
{
    unsigned char i;

    for(i = 28; i > 16; i -= 4) {
        m_rkey[i+0] = m_rkey[i+0] ^ m_rkey[i-4];
        m_rkey[i+1] = m_rkey[i+1] ^ m_rkey[i-3];
        m_rkey[i+2] = m_rkey[i+2] ^ m_rkey[i-2];
        m_rkey[i+3] = m_rkey[i+3] ^ m_rkey[i-1];
    }

    m_rkey[16] = m_rkey[16] ^ sbox[m_rkey[12]];
    m_rkey[17] = m_rkey[17] ^ sbox[m_rkey[13]];
    m_rkey[18] = m_rkey[18] ^ sbox[m_rkey[14]];
    m_rkey[19] = m_rkey[19] ^ sbox[m_rkey[15]];

    for(i = 12; i > 0; i -= 4) {
        m_rkey[i+0] = m_rkey[i+0] ^ m_rkey[i-4];
        m_rkey[i+1] = m_rkey[i+1] ^ m_rkey[i-3];
        m_rkey[i+2] = m_rkey[i+2] ^ m_rkey[i-2];
        m_rkey[i+3] = m_rkey[i+3] ^ m_rkey[i-1];
    }

    *rc = FD(*rc);
    m_rkey[0] = m_rkey[0] ^ sbox[m_rkey[29]] ^ (*rc);
    m_rkey[1] = m_rkey[1] ^ sbox[m_rkey[30]];
    m_rkey[2] = m_rkey[2] ^ sbox[m_rkey[31]];
    m_rkey[3] = m_rkey[3] ^ sbox[m_rkey[28]];
}



inline unsigned char rj_xtime_f(unsigned char x)
{
    return (x & 0x80) ? ((x << 1) ^ 0x1b) : (x << 1);
}





/*****************************************************************************************************************************************************************
 *
 *
 *    Encryption 
 *
 *
 *****************************************************************************************************************************************************************
*/

void CIBEncryption::AES_Encrypt(const unsigned char*	key, 
						  const unsigned char*	plain,		
						  const int				plain_length,		
						  unsigned char*			encrypted)
{

//	clock_t start, end;
//	start = clock();

	//------------------------------------------------------------------------------------------
	//
	// Init
	//

	int padding = 0;

	m_remainingLength= plain_length;

#if 1
	//Calculate padding
	if(m_remainingLength % BLOCK_SIZE != 0)
		padding = (BLOCK_SIZE - (m_remainingLength % BLOCK_SIZE));
	m_remainingLength += padding;
#endif 
	// Add 1 byte for padding size
	encrypted[0] = padding & 0xFF;
	
	// Reset Buffer
	m_buffer_pos = 0;
	m_encrypted_pos = 0;


	
	//------------------------------------------------------------------------------------------
	//
	// Encryption
	//
	register int total_plain_length = m_remainingLength;
	register int i;
	
	
	//#pragma omp parallel for
	for( i = 0; i < total_plain_length; i+=BLOCK_SIZE)
	{
		memcpy(m_buffer, plain+i, BLOCK_SIZE);
		check_and_encrypt_buffer(encrypted);
	}


	//------------------------------------------------------------------------------------------
	//
	// Encryption - last block
	//
	// the last block encryption, if there is remain buffer

	if (m_remainingLength > 0) {
		memset(m_buffer, 0, BLOCK_SIZE); // padding
        memcpy(m_buffer, plain+i, BLOCK_SIZE);

        encrypt(m_buffer);


		memcpy(encrypted+1+m_encrypted_pos, m_buffer, BLOCK_SIZE);
		m_encrypted_pos+=BLOCK_SIZE;
		m_remainingLength-=BLOCK_SIZE;
    }



//	end = clock();
	//TRACE("1 Block Encryption elapsed time = %f(ms)\n", (end - start) / (1088*1022 / 16.0));
//	TRACE("Elapsed encryption time = %d(ms)\t", end - start);
	
}




void CIBEncryption::check_and_encrypt_buffer(unsigned char *encrypted)
{
	// Encrypt 1 block
	encrypt(m_buffer);
	
	// Copy to output buffer
	memcpy(encrypted+m_encrypted_pos, m_buffer, BLOCK_SIZE);
	m_encrypted_pos		+= BLOCK_SIZE;
	m_remainingLength	-= BLOCK_SIZE;

}

void CIBEncryption::encrypt(unsigned char *buffer)
{

	

#if 0
	//copy_key();

	add_round_key(buffer, 0);
	register unsigned char round;
	// round 1~13
	for(round = 1 ; round < NUM_ROUNDS; ++round)
	{
		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, round);
	}
	// round 14
	sub_bytes(buffer);
	shift_rows(buffer);
	add_round_key(buffer, round);
#else
	#if 0
		add_round_key(buffer, 0);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 1);
	
		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 2);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 3);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 4);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 5);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 6);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 7);
	
		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 8);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 9);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 10);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 11);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 12);

		sub_bytes(buffer);
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key(buffer, 13);

		sub_bytes(buffer);
		shift_rows(buffer);
		add_round_key(buffer, 14);
	#else
	
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);
	
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);
	
		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		mix_columns(buffer);
		add_round_key_sub_bytes(buffer, 0);

		shift_rows(buffer);
		add_round_key(buffer, 0);
	#endif



#endif
}

/*
void CIBEncryption::copy_key()
{
	memcpy(m_rkey, m_key, KEY_SIZE);
}
*/

void CIBEncryption::add_round_key(unsigned char *buffer, const unsigned char round)
{
#if 1
	unsigned int *pbuffer = (unsigned int *)buffer;
	unsigned int *pm_rkey_all = (unsigned int *)&m_rkey_all[round * BLOCK_SIZE];

	pbuffer[0] ^= pm_rkey_all[0];
	pbuffer[1] ^= pm_rkey_all[1];
	pbuffer[2] ^= pm_rkey_all[2];
	pbuffer[3] ^= pm_rkey_all[3];
/*
	int i_round = round * BLOCK_SIZE;
	buffer[0] = buffer[0] ^ m_rkey_all[ i_round + 0 ];
	buffer[1] = buffer[1] ^ m_rkey_all[ i_round + 1 ];
	buffer[2] = buffer[2] ^ m_rkey_all[ i_round + 2 ];
	buffer[3] = buffer[3] ^ m_rkey_all[ i_round + 3 ];
	buffer[4] = buffer[4] ^ m_rkey_all[ i_round + 4 ];
	
	buffer[5] = buffer[5] ^ m_rkey_all[ i_round + 5 ];
	buffer[6] = buffer[6] ^ m_rkey_all[ i_round + 6 ];
	buffer[7] = buffer[7] ^ m_rkey_all[ i_round + 7 ];
	buffer[8] = buffer[8] ^ m_rkey_all[ i_round + 8 ];
	buffer[9] = buffer[9] ^ m_rkey_all[ i_round + 9 ];

	buffer[10] = buffer[10] ^ m_rkey_all[ i_round + 10 ];
	buffer[11] = buffer[11] ^ m_rkey_all[ i_round + 11 ];
	buffer[12] = buffer[12] ^ m_rkey_all[ i_round + 12 ];
	buffer[13] = buffer[13] ^ m_rkey_all[ i_round + 13 ];
	buffer[14] = buffer[14] ^ m_rkey_all[ i_round + 14 ];
	buffer[15] = buffer[15] ^ m_rkey_all[ i_round + 15 ];
*/
#else
	int i_round = round * BLOCK_SIZE;

	buffer[0] = sbox[buffer[0] ^ m_rkey_all[ i_round + 0 ]];
	buffer[1] = sbox[buffer[1] ^ m_rkey_all[ i_round + 1 ]];
	buffer[2] = sbox[buffer[2] ^ m_rkey_all[ i_round + 2 ]];
	buffer[3] = sbox[buffer[3] ^ m_rkey_all[ i_round + 3 ]];
	buffer[4] = sbox[buffer[4] ^ m_rkey_all[ i_round + 4 ]];

	buffer[5] = sbox[buffer[5] ^ m_rkey_all[ i_round + 5 ]];
	buffer[6] = sbox[buffer[6] ^ m_rkey_all[ i_round + 6 ]];
	buffer[7] = sbox[buffer[7] ^ m_rkey_all[ i_round + 7 ]];
	buffer[8] = sbox[buffer[8] ^ m_rkey_all[ i_round + 8 ]];
	buffer[9] = sbox[buffer[9] ^ m_rkey_all[ i_round + 9 ]];

	buffer[10] = sbox[buffer[10] ^ m_rkey_all[ i_round + 10 ]];
	buffer[11] = sbox[buffer[11] ^ m_rkey_all[ i_round + 11 ]];
	buffer[12] = sbox[buffer[12] ^ m_rkey_all[ i_round + 12 ]];
	buffer[13] = sbox[buffer[13] ^ m_rkey_all[ i_round + 13 ]];
	buffer[14] = sbox[buffer[14] ^ m_rkey_all[ i_round + 14 ]];
	buffer[15] = sbox[buffer[15] ^ m_rkey_all[ i_round + 15 ]];
#endif

#if 0
	buffer[0] ^= m_rkey_all[ round * BLOCK_SIZE  + 0 ];
	buffer[1] ^= m_rkey_all[ round * BLOCK_SIZE  + 1 ];
	buffer[2] ^= m_rkey_all[ round * BLOCK_SIZE  + 2 ];
	buffer[3] ^= m_rkey_all[ round * BLOCK_SIZE  + 3 ];
	buffer[4] ^= m_rkey_all[ round * BLOCK_SIZE  + 4 ];
	
	buffer[5] ^= m_rkey_all[ round * BLOCK_SIZE  + 5 ];
	buffer[6] ^= m_rkey_all[ round * BLOCK_SIZE  + 6 ];
	buffer[7] ^= m_rkey_all[ round * BLOCK_SIZE  + 7 ];
	buffer[8] ^= m_rkey_all[ round * BLOCK_SIZE  + 8 ];
	buffer[9] ^= m_rkey_all[ round * BLOCK_SIZE  + 9 ];

	buffer[10] ^= m_rkey_all[ round * BLOCK_SIZE  + 10 ];
	buffer[11] ^= m_rkey_all[ round * BLOCK_SIZE  + 11 ];
	buffer[12] ^= m_rkey_all[ round * BLOCK_SIZE  + 12 ];
	buffer[13] ^= m_rkey_all[ round * BLOCK_SIZE  + 13 ];
	buffer[14] ^= m_rkey_all[ round * BLOCK_SIZE  + 14 ];
	buffer[15] ^= m_rkey_all[ round * BLOCK_SIZE  + 15 ];
#endif
}


void CIBEncryption::add_round_key_sub_bytes(unsigned char *buffer, const unsigned char round)
{

	int i_round = round * BLOCK_SIZE;

	buffer[0] = sbox[buffer[0] ^ m_rkey_all[ i_round + 0 ]];
	buffer[1] = sbox[buffer[1] ^ m_rkey_all[ i_round + 1 ]];
	buffer[2] = sbox[buffer[2] ^ m_rkey_all[ i_round + 2 ]];
	buffer[3] = sbox[buffer[3] ^ m_rkey_all[ i_round + 3 ]];
	buffer[4] = sbox[buffer[4] ^ m_rkey_all[ i_round + 4 ]];

	buffer[5] = sbox[buffer[5] ^ m_rkey_all[ i_round + 5 ]];
	buffer[6] = sbox[buffer[6] ^ m_rkey_all[ i_round + 6 ]];
	buffer[7] = sbox[buffer[7] ^ m_rkey_all[ i_round + 7 ]];
	buffer[8] = sbox[buffer[8] ^ m_rkey_all[ i_round + 8 ]];
	buffer[9] = sbox[buffer[9] ^ m_rkey_all[ i_round + 9 ]];

	buffer[10] = sbox[buffer[10] ^ m_rkey_all[ i_round + 10 ]];
	buffer[11] = sbox[buffer[11] ^ m_rkey_all[ i_round + 11 ]];
	buffer[12] = sbox[buffer[12] ^ m_rkey_all[ i_round + 12 ]];
	buffer[13] = sbox[buffer[13] ^ m_rkey_all[ i_round + 13 ]];
	buffer[14] = sbox[buffer[14] ^ m_rkey_all[ i_round + 14 ]];
	buffer[15] = sbox[buffer[15] ^ m_rkey_all[ i_round + 15 ]];
}



void CIBEncryption::sub_bytes(unsigned char *buffer)
{
	buffer[0] = sbox[buffer[0]];
	buffer[1] = sbox[buffer[1]];
	buffer[2] = sbox[buffer[2]];
	buffer[3] = sbox[buffer[3]];
	buffer[4] = sbox[buffer[4]];
	buffer[5] = sbox[buffer[5]];
	buffer[6] = sbox[buffer[6]];
	buffer[7] = sbox[buffer[7]];
	buffer[8] = sbox[buffer[8]];
	buffer[9] = sbox[buffer[9]];

	buffer[10] = sbox[buffer[10]];
	buffer[11] = sbox[buffer[11]];
	buffer[12] = sbox[buffer[12]];
	buffer[13] = sbox[buffer[13]];
	buffer[14] = sbox[buffer[14]];
	buffer[15] = sbox[buffer[15]];
}


void CIBEncryption::shift_rows(unsigned char *buffer)
{
	register unsigned char i, j, k, l;  // to make it potentially parallelable

	i			= buffer[1];
	buffer[1]	= buffer[5];
	buffer[5]	= buffer[9];
	buffer[9]	= buffer[13];
	buffer[13]	= i;

	j			= buffer[10];
	buffer[10]	= buffer[2];
	buffer[2]	= j;

	k			= buffer[3];
	buffer[3]	= buffer[15];
	buffer[15]	= buffer[11];
	buffer[11]	= buffer[7];
	buffer[7]	= k;

	l			= buffer[14];
	buffer[14]	= buffer[6];;
	buffer[6]	= l;
}

void CIBEncryption::add_round_key_sub_bytes_shift_rows(unsigned char *buffer, const unsigned char round)
{
	int i_round = round * BLOCK_SIZE;
	/*buffer[0]	= sbox[buffer[0]^m_rkey_all[ i_round + 0 ]];
	buffer[13]	= sbox[buffer[1]^m_rkey_all[ i_round + 1 ]];
	buffer[10]	= sbox[buffer[2]^m_rkey_all[ i_round + 2 ]];
	buffer[7]	= sbox[buffer[3]^m_rkey_all[ i_round + 3 ]];
	buffer[4]	= sbox[buffer[4]^m_rkey_all[ i_round + 4 ]];
	
	buffer[1]	= sbox[buffer[5]^m_rkey_all[ i_round  + 5 ]];
	buffer[14]	= sbox[buffer[6]^m_rkey_all[ i_round  + 6 ]];
	buffer[11]	= sbox[buffer[7]^m_rkey_all[ i_round  + 7 ]];
	buffer[8]	= sbox[buffer[8]^m_rkey_all[ i_round  + 8 ]];
	buffer[5]	= sbox[buffer[9]^m_rkey_all[ i_round  + 9 ]];

	buffer[2]	= sbox[buffer[10]^m_rkey_all[ i_round  + 10 ]];
	buffer[15]	= sbox[buffer[11]^m_rkey_all[ i_round  + 11 ]];
	buffer[12]	= sbox[buffer[12]^m_rkey_all[ i_round  + 12 ]];
	buffer[13]	= sbox[buffer[13]^m_rkey_all[ i_round  + 13 ]];
	buffer[6]	= sbox[buffer[14]^m_rkey_all[ i_round  + 14 ]];
	buffer[3]	= sbox[buffer[15]^m_rkey_all[ i_round  + 15 ]];*/

	buffer[0]	= sbox[buffer[0]^m_rkey_all[ i_round + 0 ]];
	buffer[1]	= sbox[buffer[5]^m_rkey_all[ i_round  + 5 ]];
	buffer[2]	= sbox[buffer[10]^m_rkey_all[ i_round  + 10 ]];
	buffer[3]	= sbox[buffer[15]^m_rkey_all[ i_round  + 15 ]];
	buffer[4]	= sbox[buffer[4]^m_rkey_all[ i_round + 4 ]];


	buffer[5]	= sbox[buffer[9]^m_rkey_all[ i_round  + 9 ]];
	buffer[6]	= sbox[buffer[14]^m_rkey_all[ i_round  + 14 ]];
	buffer[7]	= sbox[buffer[3]^m_rkey_all[ i_round + 3 ]];
	buffer[8]	= sbox[buffer[8]^m_rkey_all[ i_round  + 8 ]];
	buffer[9]	= sbox[buffer[9]^m_rkey_all[ i_round  + 9 ]];

	
	buffer[10]	= sbox[buffer[2]^m_rkey_all[ i_round + 2 ]];
	buffer[11]	= sbox[buffer[7]^m_rkey_all[ i_round  + 7 ]];
	buffer[12]	= sbox[buffer[12]^m_rkey_all[ i_round  + 12 ]];
	buffer[13]	= sbox[buffer[1]^m_rkey_all[ i_round + 1 ]];
	buffer[14]	= sbox[buffer[6]^m_rkey_all[ i_round  + 6 ]];
	buffer[15]	= sbox[buffer[11]^m_rkey_all[ i_round  + 11 ]];
	
	
	

	
	
}

void CIBEncryption::mix_columns(unsigned char * buffer)
{
	register unsigned char a, b, c, d, e;


		a = buffer[0];
		b = buffer[1];
		c = buffer[2];
		d = buffer[3];

		e = a ^ b ^ c ^d;

		buffer[0] ^= e ^ rj_xtime[a^b];
		buffer[1] ^= e ^ rj_xtime[b^c];
		buffer[2] ^= e ^ rj_xtime[c^d];
		buffer[3] ^= e ^ rj_xtime[d^a];

		a = buffer[4];
		b = buffer[5];
		c = buffer[6];
		d = buffer[7];

		e = a ^ b ^ c ^d;

		buffer[4] ^= e ^ rj_xtime[a^b];
		buffer[5] ^= e ^ rj_xtime[b^c];
		buffer[6] ^= e ^ rj_xtime[c^d];
		buffer[7] ^= e ^ rj_xtime[d^a];

		a = buffer[8];
		b = buffer[9];
		c = buffer[10];
		d = buffer[11];

		e = a ^ b ^ c ^d;

		buffer[8] ^= e ^ rj_xtime[a^b];
		buffer[9] ^= e ^ rj_xtime[b^c];
		buffer[10] ^= e ^ rj_xtime[c^d];
		buffer[11] ^= e ^ rj_xtime[d^a];

		a = buffer[12];
		b = buffer[13];
		c = buffer[14];
		d = buffer[15];

		e = a ^ b ^ c ^d;

		buffer[12] ^= e ^ rj_xtime[a^b];
		buffer[13] ^= e ^ rj_xtime[b^c];
		buffer[14] ^= e ^ rj_xtime[c^d];
		buffer[15] ^= e ^ rj_xtime[d^a];
}







/*****************************************************************************************************************************************************************
 *
 *
 *    Decryption 
 *
 *
 *****************************************************************************************************************************************************************
*/


void CIBEncryption::check_and_decrypt_buffer(unsigned char *plain)
{
	decrypt(m_buffer);

	if(m_remainingLength >= BLOCK_SIZE)
	{
		memcpy(plain + m_decrypted_pos, m_buffer, BLOCK_SIZE);
		m_decrypted_pos		+= BLOCK_SIZE;
		m_remainingLength	-= BLOCK_SIZE;
	}
}

void CIBEncryption::decrypt_opt(unsigned char* in_buf, unsigned char *out_buf)
{
//	memcpy(out_buf, in_buf, BLOCK_SIZE);

	add_round_key(in_buf, 14);
	shift_rows_inv_sub_bytes_inv_add_round_key(in_buf, tmp_buf, 13);
	mix_columns_inv(tmp_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 12);
	mix_columns_inv(out_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 11);
	mix_columns_inv(tmp_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 10);
	mix_columns_inv(out_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 9);
	mix_columns_inv(tmp_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 8);
	mix_columns_inv(out_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 7);
	mix_columns_inv(tmp_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 6);
	mix_columns_inv(out_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 5);
	mix_columns_inv(tmp_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 4);
	mix_columns_inv(out_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 3);
	mix_columns_inv(tmp_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 2);
	mix_columns_inv(out_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(out_buf, tmp_buf, 1);
	mix_columns_inv(tmp_buf);

	shift_rows_inv_sub_bytes_inv_add_round_key(tmp_buf, out_buf, 0);
}

void CIBEncryption::decrypt(unsigned char* buffer)
{
	unsigned char outbuffer[BLOCK_SIZE] = {0,};

	add_round_key(buffer, 14);
	shift_rows_inv_sub_bytes_inv_add_round_key(buffer, outbuffer, 13);
	mix_columns_inv(outbuffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(outbuffer, buffer, 12);
	mix_columns_inv(buffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(buffer, outbuffer, 11);
	mix_columns_inv(outbuffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(outbuffer, buffer, 10);
	mix_columns_inv(buffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(buffer, outbuffer, 9);
	mix_columns_inv(outbuffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(outbuffer, buffer, 8);
	mix_columns_inv(buffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(buffer, outbuffer, 7);
	mix_columns_inv(outbuffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(outbuffer, buffer, 6);
	mix_columns_inv(buffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(buffer, outbuffer, 5);
	mix_columns_inv(outbuffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(outbuffer, buffer, 4);
	mix_columns_inv(buffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(buffer, outbuffer, 3);
	mix_columns_inv(outbuffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(outbuffer, buffer, 2);
	mix_columns_inv(buffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(buffer, outbuffer, 1);
	mix_columns_inv(outbuffer);

	shift_rows_inv_sub_bytes_inv_add_round_key(outbuffer, buffer, 0);
}


void CIBEncryption::shift_rows_inv(unsigned char *buffer)
{
	register unsigned char i, j, k, l;  // to make it potentially parallelable

	i			= buffer[1];
	buffer[1]	= buffer[13];
	buffer[13]	= buffer[9];
	buffer[9]	= buffer[5];
	buffer[5]	= i;

	j			= buffer[2];
	buffer[2]	= buffer[10];
	buffer[10]	= j;

	k			= buffer[3];
	buffer[3]	= buffer[7];
	buffer[7]	= buffer[11];
	buffer[11]	= buffer[15];
	buffer[15]	= k;

	l			= buffer[6];
	buffer[6]	= buffer[14];;
	buffer[14]	= l;
}

void CIBEncryption::shift_rows_inv_sub_bytes_inv_add_round_key(unsigned char *buffer, unsigned char *out_buffer, const unsigned char round)
{
	unsigned short i_round = round * BLOCK_SIZE;
	
	out_buffer[0] = sboxinv[buffer[0]] ^ m_rkey_all[ i_round + 0 ];
	out_buffer[1] = sboxinv[buffer[13]] ^ m_rkey_all[ i_round + 1 ];
	out_buffer[2] = sboxinv[buffer[10]] ^ m_rkey_all[ i_round + 2 ];
	out_buffer[3] = sboxinv[buffer[7]] ^ m_rkey_all[ i_round + 3 ];
	out_buffer[4] = sboxinv[buffer[4]] ^ m_rkey_all[ i_round + 4 ];
	
	out_buffer[5] = sboxinv[buffer[1]] ^ m_rkey_all[ i_round + 5 ];
	out_buffer[6] = sboxinv[buffer[14]] ^ m_rkey_all[ i_round + 6 ];
	out_buffer[7] = sboxinv[buffer[11]] ^ m_rkey_all[ i_round + 7 ];
	out_buffer[8] = sboxinv[buffer[8]] ^ m_rkey_all[ i_round + 8 ];
	out_buffer[9] = sboxinv[buffer[5]] ^ m_rkey_all[ i_round + 9 ];

	out_buffer[10] = sboxinv[buffer[2]] ^ m_rkey_all[ i_round + 10 ];
	out_buffer[11] = sboxinv[buffer[15]] ^ m_rkey_all[ i_round + 11 ];
	out_buffer[12] = sboxinv[buffer[12]] ^ m_rkey_all[ i_round + 12 ];
	out_buffer[13] = sboxinv[buffer[9]] ^ m_rkey_all[ i_round + 13 ];
	out_buffer[14] = sboxinv[buffer[6]] ^ m_rkey_all[ i_round + 14 ];
	out_buffer[15] = sboxinv[buffer[3]] ^ m_rkey_all[ i_round + 15 ];
}

void CIBEncryption::sub_bytes_inv_add_round_key(unsigned char *buffer, const unsigned char round)
{

	int i_round = round * BLOCK_SIZE;

	buffer[0] = sboxinv[buffer[0]] ^ m_rkey_all[ i_round + 0 ];
	buffer[1] = sboxinv[buffer[1]] ^ m_rkey_all[ i_round + 1 ];
	buffer[2] = sboxinv[buffer[2]] ^ m_rkey_all[ i_round + 2 ];
	buffer[3] = sboxinv[buffer[3]] ^ m_rkey_all[ i_round + 3 ];
	buffer[4] = sboxinv[buffer[4]] ^ m_rkey_all[ i_round + 4 ];
	
	buffer[5] = sboxinv[buffer[5]] ^ m_rkey_all[ i_round + 5 ];
	buffer[6] = sboxinv[buffer[6]] ^ m_rkey_all[ i_round + 6 ];
	buffer[7] = sboxinv[buffer[7]] ^ m_rkey_all[ i_round + 7 ];
	buffer[8] = sboxinv[buffer[8]] ^ m_rkey_all[ i_round + 8 ];
	buffer[9] = sboxinv[buffer[9]] ^ m_rkey_all[ i_round + 9 ];

	buffer[10] = sboxinv[buffer[10]] ^ m_rkey_all[ i_round + 10 ];
	buffer[11] = sboxinv[buffer[11]] ^ m_rkey_all[ i_round + 11 ];
	buffer[12] = sboxinv[buffer[12]] ^ m_rkey_all[ i_round + 12 ];
	buffer[13] = sboxinv[buffer[13]] ^ m_rkey_all[ i_round + 13 ];
	buffer[14] = sboxinv[buffer[14]] ^ m_rkey_all[ i_round + 14 ];
	buffer[15] = sboxinv[buffer[15]] ^ m_rkey_all[ i_round + 15 ];
}

void CIBEncryption::sub_bytes_inv(unsigned char *buffer)
{
	
    buffer[0] = sboxinv[buffer[0]];
	buffer[1] = sboxinv[buffer[1]];
	buffer[2] = sboxinv[buffer[2]];
	buffer[3] = sboxinv[buffer[3]];
	buffer[4] = sboxinv[buffer[4]];
	buffer[5] = sboxinv[buffer[5]];
	buffer[6] = sboxinv[buffer[6]];
	buffer[7] = sboxinv[buffer[7]];
	buffer[8] = sboxinv[buffer[8]];
	buffer[9] = sboxinv[buffer[9]];
	buffer[10] = sboxinv[buffer[10]];
	buffer[11] = sboxinv[buffer[11]];
	buffer[12] = sboxinv[buffer[12]];
	buffer[13] = sboxinv[buffer[13]];
	buffer[14] = sboxinv[buffer[14]];
	buffer[15] = sboxinv[buffer[15]];
	
}



void CIBEncryption::mix_columns_inv(unsigned char* buffer)
{
    register unsigned char a, b, c, d, e, x, y, z;

	for(int i=0; i<16; i+=4)
	{
		// 5ms
		a = buffer[i];
		b = buffer[i+1];
		c = buffer[i+2];
		d = buffer[i+3];

		// 30ms
		e = a ^ b ^ c ^ d;
		z = rj_xtime[e];
		x = e ^ rj_xtime_rj_xtime[z^a^c];
		y = e ^ rj_xtime_rj_xtime[z^b^d];

		// 20ms
		buffer[i] ^= x ^ rj_xtime[a^b];
		buffer[i+1] ^= y ^ rj_xtime[b^c];
		buffer[i+2] ^= x ^ rj_xtime[c^d];
		buffer[i+3] ^= y ^ rj_xtime[d^a];
	}
/*
	a = buffer[0];
	b = buffer[1];
	c = buffer[2];
	d = buffer[3];

	e = a ^ b ^ c ^ d;
	z = rj_xtime[e];
	x = e ^ rj_xtime_rj_xtime[z^a^c];
	y = e ^ rj_xtime_rj_xtime[z^b^d];

	buffer[0] ^= x ^ rj_xtime[a^b];
	buffer[1] ^= y ^ rj_xtime[b^c];
	buffer[2] ^= x ^ rj_xtime[c^d];
	buffer[3] ^= y ^ rj_xtime[d^a];

	
	a = buffer[4];
	b = buffer[5];
	c = buffer[6];
	d = buffer[7];

	e = a ^ b ^ c ^ d;
	z = rj_xtime[e];
	x = e ^ rj_xtime_rj_xtime[z^a^c];
	y = e ^ rj_xtime_rj_xtime[z^b^d];

	buffer[4] ^= x ^ rj_xtime[a^b];
	buffer[5] ^= y ^ rj_xtime[b^c];
	buffer[6] ^= x ^ rj_xtime[c^d];
	buffer[7] ^= y ^ rj_xtime[d^a];

	
	a = buffer[8];
	b = buffer[9];
	c = buffer[10];
	d = buffer[11];

	e = a ^ b ^ c ^ d;
	z = rj_xtime[e];
	x = e ^ rj_xtime_rj_xtime[z^a^c];
	y = e ^ rj_xtime_rj_xtime[z^b^d];

	buffer[8] ^= x ^ rj_xtime[a^b];
	buffer[9] ^= y ^ rj_xtime[b^c];
	buffer[10] ^= x ^ rj_xtime[c^d];
	buffer[11] ^= y ^ rj_xtime[d^a];


	a = buffer[12];
	b = buffer[13];
	c = buffer[14];
	d = buffer[15];

	e = a ^ b ^ c ^ d;
	z = rj_xtime[e];
	x = e ^ rj_xtime_rj_xtime[z^a^c];
	y = e ^ rj_xtime_rj_xtime[z^b^d];

	buffer[12] ^= x ^ rj_xtime[a^b];
	buffer[13] ^= y ^ rj_xtime[b^c];
	buffer[14] ^= x ^ rj_xtime[c^d];
	buffer[15] ^= y ^ rj_xtime[d^a];
*/

}

void CIBEncryption::AES_Get_AES_KEY_ALL(unsigned char* outBuff, int size)
{
	memcpy(outBuff, (unsigned char*)m_rkey_all, size);
	//memset(outBuff, 0, size);

	return;
}

void CIBEncryption::AES_Set_Key(unsigned char* key)
{
	memcpy(m_primary_key, key, KEY_SIZE);

	memset(m_rkey,		0, KEY_SIZE);
	memset(m_rkey_all,	0, 1024);
	//memset(m_rkey_dec,	0, KEY_ALL_ROUND_SIZE);

	memcpy(m_key, key, KEY_SIZE);

	// Round Key 固府 积己窍扁 - Encryption
	memcpy(m_rkey_all,	m_key, KEY_SIZE);
	memcpy(m_rkey,		m_key, KEY_SIZE);
	unsigned char rcon = 1;

	int key_index = KEY_SIZE;
    for(int i = 1 ; i < NUM_ROUNDS; ++i )
    {
        if( !(i & 1) )
		{
            expand_enc_key(&rcon);
			memcpy(m_rkey_all + key_index, m_rkey, KEY_SIZE);
			key_index += KEY_SIZE;
		}
    }
    expand_enc_key(&rcon);
	memcpy(m_rkey_all + key_index, m_rkey, KEY_SIZE);

	memcpy(m_rkey_all + 256, m_rkey_all, 256); // padding 256 byte to make 512 byte size

/*	// Round Key 固府 积己窍扁 - Decryption
	memcpy(m_rkey,		m_key, KEY_SIZE);

	rcon = 1;
	key_index = KEY_ALL_ROUND_SIZE - KEY_SIZE;

	unsigned char i;

    for (i = NUM_ROUNDS / 2; i > 0; --i)
        expand_enc_key(&rcon);

	memcpy(m_rkey_dec + key_index, m_rkey, KEY_SIZE);
	key_index -= KEY_SIZE;

    for (i = NUM_ROUNDS, rcon = 0x80; --i;)
    {
        if( (i & 1) )
		{
            expand_dec_key(&rcon);
			memcpy(m_rkey_dec + key_index, m_rkey, KEY_SIZE);
			key_index -= KEY_SIZE;
		}
    }
*/

#if 0
	FILE *fp;
	fp = fopen("d:\\round_key_all.txt", "w"); 
	for(int index = 1 ; index <= KEY_ALL_ROUND_SIZE ; index++)
	{
		if( (index-1) % 16 == 0)
			fprintf(fp, ",\n");
		else
			fprintf(fp, ",\t");

		fprintf(fp, "0x%02x", m_rkey_all[index-1]);
	}
	fclose(fp);
#endif
	return;
}

void CIBEncryption::AES_Set_Key_Random()
{
	unsigned char key[KEY_SIZE];

	srand((unsigned int)time(NULL));

	for(int i=0; i<KEY_SIZE; i++)
	{
		key[i] = rand() % 256;
	}

	AES_Set_Key(key);
}

/*****************************************************************************************************
 *
 *   RSA
 *
 *****************************************************************************************************
 */

BIG_DECIMAL CIBEncryption::CreateDecimal(unsigned char *str, int size)
{
	BIG_DECIMAL decimal;

	decimal.digit = new unsigned char [size];

	for(int i=0; i<size ;i++)
	{
		decimal.digit[i] = str[size-i-1] - 48;
	}
	decimal.size = size;
	decimal.sign = false;

	return decimal;
}

DWORD CIBEncryption::GetDigit(BIG_DECIMAL *a)
{
	DWORD	decimal = 0;
	DWORD	exp = 1;


	for(int i=0; i<a->size ;i++)
	{
		decimal += a->digit[i] * exp;
		exp *= 10;
	}
	
	if(a->sign == TRUE)
	{
		decimal *= -1;
	}

	return decimal;
}

//char* CIBEncryption::GetString(DWORD a)
//{
//	char temp[11];
//
//
//	for(int i=0; i<a->size ;i++)
//	{
//		decimal += a->digit[i] * exp;
//		exp *= 10;
//	}
//	
//	if(a->sign == TRUE)
//	{
//		decimal *= -1;
//	}
//
//	return decimal;
//}

bool CIBEncryption::IsEqual(BIG_DECIMAL *a, BIG_DECIMAL *b)
{
	if(a->size != b->size)
		return false;


	for(int i = 0; i < a->size; i++)
		if(a->digit[i] != b->digit[i])
			return false;

	return true;
}


bool CIBEncryption::IsBigger(BIG_DECIMAL *a, BIG_DECIMAL *b)
{
	if(a->size > b->size)
		return true;

	else if(a->size < b->size)
		return false;

	else
	{
		for(int i = a->size - 1; i >= 0; i--)
		{
			if(a->digit[i] > b->digit[i])
				return true;
			else if(a->digit[i] < b->digit[i])
				return false;
		}
	}
		
	return true;
}

bool CIBEncryption::IsPrimeNumber(BIG_DECIMAL *a)
{
	BIG_DECIMAL denominator, max, result;
	unsigned char *ptrForFree;

	if(a->size == 1&& a->digit[0] == 0x02)
		return true;

	if((a->digit[0] ^ 0x01) & 0x01)
		return false;

	denominator = CreateDecimal((unsigned char*)"3", 1);

	max = DIVIDE(a, &denominator);

	while(IsBigger(&max, &denominator))
	{
		result = MOD(a, &denominator);

		if(result.size == 1 && result.digit[0] == 0)
		{
			DestroyBuffer(result.digit);
			DestroyBuffer(denominator.digit);
			DestroyBuffer(max.digit);
			return false;
		}
		DestroyBuffer(result.digit);

		ptrForFree = denominator.digit;
		denominator = PlusDigit(&denominator, 0x02);
		DestroyBuffer(ptrForFree);

		ptrForFree = max.digit;
		max = DIVIDE(a, &denominator);
		DestroyBuffer(ptrForFree);
	}

	DestroyBuffer(result.digit);
	DestroyBuffer(denominator.digit);
	DestroyBuffer(max.digit);

	return true;
}



BIG_DECIMAL CIBEncryption::PlusDigit(BIG_DECIMAL *a, unsigned char digit)
{
	BIG_DECIMAL result;

	unsigned int size = a->size + 1;
	result.digit = new unsigned char [size];

	int i;
	unsigned char temp;

	result.digit[0] = a->digit[0] + digit;
	temp = result.digit[0] / 0x0A;
	result.digit[0] %= 0x0A;

	for(i = 1; i < a->size; i++)
	{
		result.digit[i] = a->digit[i] + temp;
		temp = result.digit[i] / 0x0A;
		result.digit[i] %= 0x0A;
	}

	if(temp)
	{
		result.digit[i] = temp;
		result.size = size;
	}
	else
		result.size = a->size;

	result.sign = a->sign;


	return result;
}


BIG_DECIMAL CIBEncryption::PLUS(BIG_DECIMAL *a, BIG_DECIMAL *b)
{
	BIG_DECIMAL result;

	unsigned int	min, max;
	BIG_DECIMAL		*biggerNum = a->size > b->size ? a : b;
	min = a->size > b->size ? b->size : a->size;
	max = a->size > b->size ? a->size : b->size;

	unsigned int size = max + 1;
	result.digit = new unsigned char [size];

	unsigned int i = 0;
	unsigned char temp = 0;

	for(; i < min; i++)
	{
		result.digit[i] = a->digit[i] + b->digit[i] + temp;
		if(result.digit[i] > 0x09)
			temp = 0x01;
		else
			temp = 0x00;
		result.digit[i] %= 0x0A;
	}

	for(; i < max; i++)
	{
		result.digit[i] = biggerNum->digit[i] + temp;
		if(result.digit[i] > 0x09)
			temp = 0x01;
		else
			temp = 0x00;
		result.digit[i] %= 0x0A;
	}

	if(temp)
	{
		result.digit[i] = temp;
		result.size = size;
	}
	else
		result.size = size - 1;

	result.sign = 0;

	return result;
}



BIG_DECIMAL CIBEncryption::MinusAbsolute(BIG_DECIMAL *a, BIG_DECIMAL *b)
{
	BIG_DECIMAL result;

	result.digit = new unsigned char [a->size];

	int i = 0;

	unsigned char temp = 0;

	for(; i <  b->size; i++)
	{
		if(a->digit[i] >= b->digit[i] + temp)
		{
			result.digit[i] = a->digit[i] - b->digit[i] - temp;
			temp = 0;
		}
		else
		{
			result.digit[i] = a->digit[i] + 10 - b->digit[i] - temp;
			temp = 1;
		}
	}

	for(; i < a->size; i++)
	{
		if(a->digit[i] >= temp)
		{
			result.digit[i] = a->digit[i] - temp;
			temp = 0;
		}
		else
		{
			result.digit[i] = a->digit[i] + 10 - temp;
			temp = 1;
		}
	}

	result.size = a->size;

	while(!result.digit[i-1] && i > 1)
	{
		result.size--;
		i--;
	}

	result.sign = 0;

	return result;
}



BIG_DECIMAL CIBEncryption::MinusDigit(BIG_DECIMAL *a, unsigned char digit)
{
	BIG_DECIMAL result;

	result.digit = new unsigned char [a->size];

	int i;
	unsigned char temp = 0x00;

	if(a->digit[0] >= digit)
		result.digit[0] = a->digit[0] - digit;
	else
	{
		result.digit[0] = a->digit[0] + 10 - digit;
		temp = 1;
	}

	for(i = 1; i < a->size; i++)
	{
		if(a->digit[i] >=temp)
		{
			result.digit[i] = a->digit[i] - temp;
			temp = 0;
		}
		else
		{
			result.digit[i] = a->digit[i] + 10 - temp;
			temp = 1;
		}
	}

	result.size = a->size;

	while(!result.digit[i-1] && i > 1)
	{
		result.size--;
		i--;
	}

	result.sign = false;

	return result;
}


BIG_DECIMAL CIBEncryption::MINUS(BIG_DECIMAL *a, BIG_DECIMAL *b)
{
	BIG_DECIMAL result;

	if(IsBigger(a, b))
	{
		result = MinusAbsolute(a, b);
		result.sign = 0;
	}
	else
	{
		result = MinusAbsolute(b, a);
		result.sign = 1;
	}

	return result;
}


BIG_DECIMAL CIBEncryption::MultiplyDigit(BIG_DECIMAL *a, unsigned digit)
{
	BIG_DECIMAL result;


	unsigned int size = a->size + 1;
	result.digit = new unsigned char [size];//(unsigned char*)malloc(size);

	int i;
	unsigned char temp = 0;

	for(i = 0; i < a->size; i++)
	{
		result.digit[i] = (a->digit[i] * digit) + temp;

		temp = result.digit[i] / 0x0A;
		result.digit[i] %= 0x0A;
	}

	if(temp)
	{
		result.digit[i] = temp;
		result.size = size;
	}
	else
		result.size = a->size;

	result.sign = false;

	return result;
}


BIG_DECIMAL CIBEncryption::MULTIPLY(BIG_DECIMAL *a, BIG_DECIMAL *b)
{
	BIG_DECIMAL result, temp, tempDigit;
	unsigned char *ptrTemp;

	result = CreateDecimal((unsigned char*) "0", 1);

	temp.digit = new unsigned char [a->size + b->size];

	int i, j;


	for(i = 0; i < b->size; i++)
	{
		tempDigit = MultiplyDigit(a, b->digit[i]);

		for(j = 0; j < i; j++)
			temp.digit[j] = 0x00;
		for(;j < tempDigit.size+i; j++)
			temp.digit[j] = tempDigit.digit[j-i];
		temp.size =  tempDigit.size + i;

		free(tempDigit.digit);

		ptrTemp = result.digit;
		result = PLUS(&result, &temp);
		free(ptrTemp);
	}

	free(temp.digit);

	return result;
}


void CIBEncryption::MinusForDivide(BIG_DECIMAL *a, BIG_DECIMAL*b)
{
	int i;
	unsigned char temp = 0;
	for(i = 0; i < b->size; i++)
	{
		if(a->digit[i] >= (b->digit[i] + temp))
		{
			a->digit[i] = a->digit[i] - b->digit[i] - temp;
			temp = 0;
		}
		else
		{
			a->digit[i] = a->digit[i] + 0x0A - b->digit[i] - temp;
			temp = 0x01;
		}
	}

	for(; i < a->size; i++)
	{
		if(a->digit[i] >= temp)
		{
			a->digit[i] = a->digit[i] - temp;
			temp = 0;
		}
		else
		{
			a->digit[i] = a->digit[i] + 0x0A - temp;
			temp =  0x01;
		}
	}

	while(!a->digit[i-1] && i > 1)
	{
		a->size--;
		i--;
	}
}



BIG_DECIMAL CIBEncryption::DIVIDE(BIG_DECIMAL *a, BIG_DECIMAL *b)
{
	BIG_DECIMAL result;
	int i, j, count;

	if(!IsBigger(a,b))
	{
		result = CreateDecimal((unsigned char *) "0", 1);
		return result;
	}

	BIG_DECIMAL numerator;

	numerator.digit = new unsigned char [a->size];
	numerator.size = a->size;

	for(j = 0; j < numerator.size; j++)
		numerator.digit[j] = a->digit[j];

	i = a->size - b->size;
	result.digit = new unsigned char [i+1];

	for(j = 0; j <=i; j++)
		result.digit[j] = 0x00;

	unsigned char* ptrForOrigin = numerator.digit;

	numerator.digit += (numerator.size - b->size);
	numerator.size = b->size;

	for(; i >= 0; i--)
	{
		count = 0;

		while(IsBigger(&numerator, b))
		{
			count++;
			MinusForDivide(&numerator, b);
		}
		result.digit[i] = (unsigned char)count;

		numerator.digit--;

		if(numerator.digit[numerator.size] != 0x00)
			numerator.size++;
	}
	free(ptrForOrigin);

	i = a->size - b->size;

	for(; i > 0; i--)
		if(result.digit[i] != 0x00)
			break;

	result.size = i + 1;
	result.sign = 0;

	return result;
}




BIG_DECIMAL CIBEncryption::MOD(BIG_DECIMAL *a, BIG_DECIMAL *m)
{
	BIG_DECIMAL result;
	int i;

	result.digit = /*(unsigned char*)malloc(a->size);/*/new unsigned char [a->size];
	for(i = 0; i< a->size; i++)
		result.digit[i] = a->digit[i];
	result.size = a->size;
	result.sign = 0;

	if(!IsBigger(a, m))
		return result;

	unsigned char* ptrForOrigin = result.digit;
	

	result.digit += (result.size - m->size);
	result.size = m->size;
	

	for(i = (a->size - m->size); i >= 0 ; i--)
	{
		while(IsBigger(&result, m))
			MinusForDivide(&result, m);
		
		result.digit--;

		if(result.digit[result.size] != 0)
			result.size++;
	}

	result.digit = ptrForOrigin;

	i = a->size - 1;

	while(i > 0)
	{
		if(result.digit[i] != 0x00)
			break;
		i--;
	}
	result.size = i + 1;

	m_ptrForOrigin = ptrForOrigin;
	//delete [] ptrForOrigin;

	return result;
}


BIG_DECIMAL CIBEncryption::MULTIPLY_EXPONENT(BIG_DECIMAL *a, BIG_DECIMAL *e)
{
	int i, j, position;
	unsigned char flag, *ptrForFree;
	BIG_DECIMAL result, temp;

	BIG_BINARY binaryE = GetBinary(e);

	result = CreateDecimal((unsigned char*)"1", 1);
	temp = MultiplyDigit(a, 1);

	position = 8 * (binaryE.size - 1);

	j = 8;
	flag = 0x80;

	for(i = 0; i < 8; i++)
	{
		if(binaryE.byte[binaryE.size - 1] & flag)
		{
			position += j;
			break;
		}

		j--;
		flag >>= 1;
	}

	for(i = 0; i < binaryE.size; i++)
	{
		flag = 0x01;

		for(j = 0; j < 8; j++)
		{
			if(binaryE.byte[i] & flag)
			{
				ptrForFree = result.digit;
				result = MULTIPLY(&result, &temp);
				free(ptrForFree);
			}

			position--;
			if(position == 0)
				break;

			ptrForFree = temp.digit;
			temp = MULTIPLY(&temp, &temp);
			free(ptrForFree);

			flag <<= 1;
		}
	}

	DestroyBuffer(binaryE.byte);

	return result;
}


BIG_DECIMAL CIBEncryption::MOD_EXPONENT(BIG_DECIMAL *a, BIG_DECIMAL *e, BIG_DECIMAL*m)
{
	int i, j, position;
	unsigned char flag, *ptrForFree;
	BIG_DECIMAL result, temp;

	BIG_BINARY binaryE = GetBinary(e);
	result = CreateDecimal((unsigned char*)"1", 1);
	temp = MultiplyDigit(a, 1);

	position = 8 * (binaryE.size - 1);

	j = 8;
	flag = 0x80;
#if 1
	for(i=0; i < 8; i++)
	{
		if(binaryE.byte[binaryE.size - 1] & flag)
		{
			position += j;
			break;
		}

		j--;
		flag >>= 1;
	}

	for(i = 0; i < binaryE.size; i++)
	{
		flag = 0x01;

		for(j = 0; j < 8; j++)
		{
			if(binaryE.byte[i] & flag)
			{
				ptrForFree = result.digit;
				result = MULTIPLY(&result, &temp);
				DestroyBuffer(ptrForFree);

				ptrForFree = result.digit;
				result = MOD(&result, m);
				DestroyBuffer(ptrForFree);
			}

			position--;
			if(position == 0)
				break;

			ptrForFree = temp.digit;
			temp = MULTIPLY(&temp, &temp);
			DestroyBuffer(ptrForFree);

			ptrForFree = temp.digit;
			temp = MOD(&temp, m);
			DestroyBuffer(ptrForFree);

			flag <<= 1;
			}
		}
#endif

	DestroyBuffer(binaryE.byte);
	DestroyBuffer(temp.digit);

	return result;

}



BIG_BINARY CIBEncryption::GetBinary(BIG_DECIMAL *d)
{
	int i, j, position;
	BIG_BINARY b;
	BIG_DECIMAL numerator, denominator, remainder, zero;

	b.byte = new unsigned char [(int)(d->size / 2) + 1];//(unsigned char*)malloc((int)(d->size / 2) + 1);

	numerator.digit = new unsigned char[d->size];
	numerator.size = d->size;
	for(i = 0; i < numerator.size; i++)
		numerator.digit[i] = d->digit[i];

	zero = CreateDecimal((unsigned char *)"0", 1);

	denominator = CreateDecimal((unsigned char*)"256", 3);

	for(position = 0; ;position++)
	{
		remainder = MOD(&numerator, &denominator);

		unsigned char *ptrDigit = numerator.digit;
		numerator = DIVIDE(&numerator, &denominator);
		DestroyBuffer(ptrDigit);

		b.byte[position] = 0x00;

		for(i = 0; i < remainder.size; i++)
		{
			unsigned char tempMultiply = 1;

			for(j = 0; j < i; j++)
				tempMultiply *= 10;

			b.byte[position] += remainder.digit[i] * tempMultiply;
		}

		if(IsEqual(&numerator, &zero))
			break;

		DestroyBuffer(remainder.digit);
	}

	b.size = position + 1;

	DestroyBuffer(remainder.digit);
	DestroyBuffer(numerator.digit);
	DestroyBuffer(denominator.digit);
	DestroyBuffer(zero.digit);

	return b;
}




BIG_DECIMAL CIBEncryption::RSAGetPublicKey(BIG_DECIMAL *p, BIG_DECIMAL*q)
{
	return MULTIPLY(p, q);
}


BIG_DECIMAL CIBEncryption::RSAGetSecretKey(BIG_DECIMAL *p, BIG_DECIMAL *q, BIG_DECIMAL *e)
{
	BIG_DECIMAL secretKey, temp, temp1, n, one;
	unsigned char *ptrForFree;

	one = CreateDecimal((unsigned char *)"1", 1);

	temp = MinusDigit(p, 1);
	temp1 = MinusDigit(q, 1);
	n = MULTIPLY(&temp, &temp1);

	secretKey = DIVIDE(&n, e);

	temp = MULTIPLY(e, &secretKey);

	for(;;)
	{
		ptrForFree = temp.digit;
		temp = MOD(&temp, &n);
		free(ptrForFree);

		if(IsEqual(&temp, &one))
			break;

		ptrForFree = secretKey.digit;
		secretKey = PlusDigit(&secretKey, 1);
		free(ptrForFree);

		ptrForFree= temp.digit;
		temp = PLUS(&temp, e);
		free(ptrForFree);
	}

	return secretKey;
}


BIG_DECIMAL CIBEncryption::RSAEncrypt(
									   BIG_DECIMAL *plain,
									   BIG_DECIMAL *e, 
									   BIG_DECIMAL *publicKey)
{

	return MOD_EXPONENT(plain, e, publicKey);
}


BIG_DECIMAL CIBEncryption::RSADecrypt( BIG_DECIMAL *cipher,
									   BIG_DECIMAL *secretKey,
									   BIG_DECIMAL *publicKey)
{
	return MOD_EXPONENT(cipher, secretKey, publicKey);
}


void CIBEncryption::TRACE_Decimal(BIG_DECIMAL decimal)
{
	int i;

	if(decimal.sign)
		TRACE("-");

	for(i = decimal.size - 1; i >= 0; i--)
	{
		TRACE("%c", decimal.digit[i]+48);
	}
	TRACE("\n");
}

void CIBEncryption::DestroyBuffer(unsigned char *buffer)
{
	if(buffer != NULL)
	{
		delete [] buffer;
		buffer = NULL;
	}
}



void CIBEncryption::AES_Key_Encrypt(unsigned char *in, unsigned char *out)
{
	int BLOCK_COUNT = 256;// / 2;
	DWORD dec = 0;
	char temp[6]={0,};
	BIG_DECIMAL encrypted, plain;

	for(int i = 0, j = 0; i < BLOCK_COUNT; i++, j +=2)
	{
		memset(temp, 0, 6);

		dec =	in[i];
		
		sprintf(temp,"%05u", dec);
		
		plain				= CreateDecimal((unsigned char*)temp, 5);
		encrypted			= RSAEncrypt(&plain, &m_E, &m_PublicKey);
		dec					= GetDigit(&encrypted);

		out[j+0] = (dec >> 8) & 0xFF;
		out[j+1] = dec & 0xFF;

		DestroyBuffer(plain.digit);
		DestroyBuffer(encrypted.digit);
	}

	return;
}


void CIBEncryption::AES_Key_Decrypt(unsigned char *in, unsigned char *out)
{
	int BLOCK_COUNT = 512;
	DWORD dec = 0;
	char temp[6]={0,};
	BIG_DECIMAL cipher, decrypted;

	for(int i = 0, j = 0; i < BLOCK_COUNT; i +=2, j++)
	{
		memset(temp, 0, 6);

		dec =	in[i+0] << 8 |
				in[i+1];
		
		sprintf(temp,"%05u", (unsigned int)dec);
		
		cipher				= CreateDecimal((unsigned char*)temp, 5);
		decrypted			= RSADecrypt(&cipher, &m_D, &m_PublicKey);
		dec					= GetDigit(&decrypted);


		out[j] = dec & 0xFF;

		DestroyBuffer(cipher.digit);
		DestroyBuffer(decrypted.digit);
	}


	return;
}