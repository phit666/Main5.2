#ifndef SESSIONCRYPTOR_H
#define SESSIONCRYPTOR_H

#pragma once
#include "mu_win_compat.h"

class AbstractCipher
{
public:
    AbstractCipher()
    {
    }

    virtual ~AbstractCipher()
    {
    }

    int GetMaxRunCount() { return this->m_maxRunCount; }

	virtual bool Init(const BYTE *key, DWORD length) = 0;
    virtual int Encrypt(const BYTE *inBuf, DWORD inLen, BYTE *outBuf) = 0;
    virtual int Decrypt(const BYTE *inBuf, DWORD inLen, BYTE *outBuf) = 0;

	int m_maxRunCount;
};


template <class ALGORITHM, int maxRunCount> class ConcreteCipher : public AbstractCipher
{

public:

    ConcreteCipher()
    {
    }

    virtual ~ConcreteCipher()
    {
    }

    bool Init(const BYTE *key, DWORD length)
    {
        this->enc_.SetKey(key, this->enc_.DEFAULT_KEYLENGTH);
        this->dec_.SetKey(key, this->enc_.DEFAULT_KEYLENGTH);
        this->m_maxRunCount = maxRunCount;
        return true;
    }

    int Encrypt(const BYTE *inBuf, DWORD inLen, BYTE *outBuf)
    {
        int paddingSize = 0;
        int remainder = inLen % this->enc_.BLOCKSIZE;

        if (remainder != 0)
        {
            paddingSize = this->enc_.BLOCKSIZE - remainder;
        }

        int outLen = inLen + paddingSize + 1;

        if (outBuf != NULL && outLen >= 1 && outLen <= 4096)
        {
            BYTE padding[64];
            BYTE obuf[4096];
            BYTE tbuf[4096];


            memset(padding, 0, sizeof(padding));
            memset(tbuf, 0, outLen);
            memset(obuf, 0, outLen);


            memcpy(tbuf, inBuf, inLen);
            memcpy(&tbuf[inLen], padding, paddingSize);


            for (int i = 0; i < outLen; i += this->enc_.BLOCKSIZE)
            {
                this->enc_.ProcessBlock(&tbuf[i], &obuf[i]);
            }

            obuf[outLen - 1] = paddingSize;
            memcpy(outBuf, obuf, outLen);
        }

        return outLen;
    }

    int Decrypt(const BYTE *inBuf, DWORD inLen, BYTE *outBuf)
    {
        int remainder = inLen % this->dec_.BLOCKSIZE;
        int outLen = 0;


        if (remainder == 1)
		{
            int paddingSize = inBuf[inLen - 1];
            outLen = inLen - paddingSize - 1;


            if (outBuf != NULL)
            {
                if (outLen > 0 && outLen <= 4096)
                {
                    BYTE obuf[4096];
                    BYTE ibuf[4096];


                    memset(ibuf, 0, inLen);
                    memset(obuf, 0, inLen);


                    memcpy(ibuf, inBuf, inLen);


                    for (int i = 0; i < outLen; i += this->dec_.BLOCKSIZE)
                    {
                        this->dec_.ProcessBlock(&ibuf[i], &obuf[i]);
                    }


                    memcpy(outBuf, obuf, outLen);
                }
            }

            return outLen;
        }

        return -1;
    }


private:

    typename ALGORITHM::Encryption enc_;
    typename ALGORITHM::Decryption dec_;
};


class CCryptoModulus
{
public:
    CCryptoModulus();
    virtual ~CCryptoModulus();
    
    bool InitCrypto(BYTE *temp, DWORD length);
    bool InitCrypto(DWORD algorithm, BYTE *key, DWORD keyLength);


    DWORD GetAlgorithm() { return this->m_algorithm; }


    int Encrypt(void *lpTarget, void *lpSource, int iSize);
    int Decrypt(void *lpTarget, void *lpSource, int iSize);


    int GetMaxRunCount() { return this->m_cipher->GetMaxRunCount(); }


    bool LoadEncryptionKey(char *lpszFileName);
    bool LoadDecryptionKey(char *lpszFileName);


private:
    AbstractCipher * m_cipher;
    DWORD m_algorithm;
};


class CSessionCryptor
{
public:

    class Cryptor
	{
    public:

        Cryptor()
        {
            this->index = 0;
            this->encCount = 1;
            this->decCount = 1;
            this->callCount = 0;
            memset(this->data, 0x63, sizeof(this->data));
        }


        virtual ~Cryptor()
        {
        }

        int index;
        int encCount;
        int decCount;
        BYTE data[128];
        int callCount;
        CCryptoModulus crypto;
    };


    CSessionCryptor();
    virtual ~CSessionCryptor();

	void InitKey(std::string key);


    bool Open(int index);
    void ForceChange(int index, void *data, int iSize);
	void EnableEncryptChange() { this->m_bEnableEncryptChange = true; };
	void DisableEncryptChange() { this->m_bEnableEncryptChange = false; };

    int Encrypt(int index, void *lpTarget, void *lpSource, int iSize);
    int Decrypt(int index, void *lpTarget, void *lpSource, int iSize);

    void Close(int index);



    int GetEncryptCount(int index);
    int GetDecryptCount(int index);
    int GetAlgorithm(int index);

    void updateData(Cryptor *cryptor, BYTE *buf, int size);
    void changeAlgorithm(Cryptor *cryptor);

    void cleanup();

private:

   std::map<int,Cryptor *> m_cryptors;
   std::string m_defaultData;
   bool m_bEnableEncryptChange;
};


extern CSessionCryptor g_CryptoSessionCS;
extern CSessionCryptor g_CryptoSessionSC;

//#define NEW_ENCDEC 1

#endif