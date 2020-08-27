#ifndef IB_ENCRYPTION_H
#define IB_ENCRYPTION_H

#include "IBSU_Global_Def.h"

#define BLOCK_SIZE 16			// 256bit

#define KEY_SIZE   32
#define KEY_ALL_ROUND_SIZE   KEY_SIZE * 8
#define NUM_ROUNDS 14

struct BIG_DECIMAL{
	unsigned char *digit;
	int size;
	bool sign;
};

struct BIG_BINARY{
	unsigned char *byte;
	int size;
};

extern const unsigned char AES_KEY_ENCRYPTION[256];

class CIBEncryption {

    public:
        CIBEncryption(const unsigned char* key);
        ~CIBEncryption();

        // AES
		void AES_Encrypt(const unsigned char* key, const unsigned char* plain,		const int plain_length,		unsigned char* encrypted);
		
		void AES_Decrypt(const unsigned char* key, unsigned char* encrypted, const unsigned int encrypted_length, unsigned char* plain);
		void AES_Decrypt_opt(unsigned char *encrypted, unsigned int encrypted_length, unsigned char *plain);
		void AES_Get_AES_KEY_ALL(unsigned char* outBuff, int size);
		void AES_Set_Key(unsigned char* key);
		void AES_Set_Key_Random();


		// RSA
		BIG_DECIMAL		m_P, m_Q, m_E;
		BIG_DECIMAL		m_PublicKey, m_D;
		BIG_DECIMAL		m_Plain, m_Decrypted;
		unsigned char	*m_ptrForOrigin;


		BIG_DECIMAL		CreateDecimal(unsigned char *str, int size);
		bool			IsEqual(BIG_DECIMAL *a, BIG_DECIMAL *b);
		bool			IsBigger(BIG_DECIMAL *a, BIG_DECIMAL *b);
		bool			IsPrimeNumber(BIG_DECIMAL *a);

		BIG_DECIMAL		PlusDigit(BIG_DECIMAL *a, unsigned char digit);
		BIG_DECIMAL		MinusAbsolute(BIG_DECIMAL *a, BIG_DECIMAL *b);
		BIG_DECIMAL		MinusDigit(BIG_DECIMAL *a, unsigned char digit);
		BIG_DECIMAL		MultiplyDigit(BIG_DECIMAL *a, unsigned digit);
		void			MinusForDivide(BIG_DECIMAL *a, BIG_DECIMAL*b);
		BIG_DECIMAL		PLUS(BIG_DECIMAL *a, BIG_DECIMAL *b);
		BIG_DECIMAL		MINUS(BIG_DECIMAL *a, BIG_DECIMAL *b);
		BIG_DECIMAL		MOD(BIG_DECIMAL *a, BIG_DECIMAL *m);
		BIG_DECIMAL		MULTIPLY(BIG_DECIMAL *a, BIG_DECIMAL *b);
		BIG_DECIMAL		DIVIDE(BIG_DECIMAL *a, BIG_DECIMAL *b);
		BIG_DECIMAL		MULTIPLY_EXPONENT(BIG_DECIMAL *a, BIG_DECIMAL *e);
		BIG_DECIMAL		MOD_EXPONENT(BIG_DECIMAL *a, BIG_DECIMAL *e, BIG_DECIMAL*m);
		BIG_BINARY		GetBinary(BIG_DECIMAL *d);

		BIG_DECIMAL		RSAGetPublicKey(BIG_DECIMAL *p, BIG_DECIMAL*q);
		BIG_DECIMAL		RSAGetSecretKey(BIG_DECIMAL *p, BIG_DECIMAL *q, BIG_DECIMAL *e);
		BIG_DECIMAL		RSAEncrypt(BIG_DECIMAL *plain, BIG_DECIMAL *e, BIG_DECIMAL *publicKey);
		BIG_DECIMAL		RSADecrypt(BIG_DECIMAL *cipher, BIG_DECIMAL *secretKey, BIG_DECIMAL *publicKey);

		void			TRACE_Decimal(BIG_DECIMAL decimal);
		void			AES_Key_Encrypt(unsigned char *in, unsigned char *out);
		void			AES_Key_Decrypt(unsigned char *in, unsigned char *out);
		DWORD			GetDigit(BIG_DECIMAL *a);
		void			DestroyBuffer(unsigned char *buffer);
		

		unsigned char		m_primary_key[KEY_SIZE*8];
		unsigned char		m_rkey_all[1024];

    private:
		// AES
		unsigned char		m_key[KEY_SIZE];
		unsigned char		m_salt[KEY_SIZE];
		unsigned char		m_rkey[KEY_SIZE];
		//unsigned char		m_rkey_dec[KEY_ALL_ROUND_SIZE];

		unsigned char		tmp_buf[BLOCK_SIZE];
		unsigned char		m_buffer[BLOCK_SIZE];
		unsigned char		m_buffer_pos;
		int					m_encrypted_pos;
		int					m_decrypted_pos;
        int					m_remainingLength;

        bool				m_decryptInitialized;

        void check_and_encrypt_buffer(unsigned char* encrypted);
        void check_and_decrypt_buffer(unsigned char* plain);

        void encrypt(unsigned char *buffer);
        void decrypt(unsigned char *buffer);
        void decrypt_opt(unsigned char *in_buf, unsigned char *out_buf);


        void expand_enc_key(unsigned char *rc);
        void expand_dec_key(unsigned char *rc);

        void sub_bytes(unsigned char *buffer);
        void sub_bytes_inv(unsigned char *buffer);


        void add_round_key(unsigned char *buffer, const unsigned char round);

		void add_round_key_sub_bytes(unsigned char *buffer, const unsigned char round);
		void sub_bytes_inv_add_round_key(unsigned char *buffer, const unsigned char round);
		void shift_rows_inv_sub_bytes_inv_add_round_key(unsigned char *buffer, unsigned char *out_buffer, const unsigned char round);

		void add_round_key_sub_bytes_shift_rows(unsigned char *buffer, const unsigned char round);
		void add_round_key_shift_rows_inv(unsigned char *buffer, const unsigned char round);
		

        void shift_rows(unsigned char *buffer);
        void shift_rows_inv(unsigned char *buffer);

        void mix_columns(unsigned char *buffer);
        void mix_columns_inv(unsigned char *buffer);

		

		// RSA


};
#endif
