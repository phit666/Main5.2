#include "stdafx.h"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1


#include "cryptlib.h"
#include "sha.h"
#include "md4.h"
#include "md5.h"
#include "ripemd.h"
#include "3way.h"
#include "cast.h"
#include "rc5.h"
#include "rc6.h"
#include "mars.h"
#include "idea.h"
#include "gost.h"
#include "tea.h"
#include "des.h"
#include "twofish.h"
#include "skipjack.h"
#include "safer.h"



#include "ComplexModulus.h"


CCryptoModulus::CCryptoModulus()
{
    this->m_cipher = NULL;
}


CCryptoModulus::~CCryptoModulus()
{
    if (this->m_cipher != NULL)
    {
        delete this->m_cipher;
        this->m_cipher = NULL;
    }
}


bool CCryptoModulus::InitCrypto(unsigned char *temp, unsigned int length)
{
    int hashid = 0;
    unsigned char digest[2048];

    memset(digest, 0, sizeof(digest));

	//"98132003732920532106543021784355"

    if (length >= 10)
    {
        //hashid = temp[4] ^ (temp[3] | ((temp[1] ^ temp[0]) < temp[2] % 3));
		hashid = temp[0]; // 9
		hashid ^= temp[1]; // 1
		hashid = hashid < temp[2] % 3; // 2
		hashid |= temp[3]; // 3
		hashid ^= temp[4]; // 6

    }
    else
    {
        hashid = temp[0];
    }


    hashid = (hashid % 11) - 1;

    switch (hashid)
    {
        case 0:
        {
            CryptoPP::SHA256 hash;
            hash.Update(temp, length);
            hash.Final(digest);


            this->m_algorithm = digest[0] % 10;
            this->InitCrypto(this->m_algorithm, digest, 32);
        }
        break;


        case 2:
        {
            CryptoPP::SHA512 hash;
            hash.Update(temp, length);
            hash.Final(digest);


            this->m_algorithm = digest[1] % 10;
            this->InitCrypto(this->m_algorithm, digest, 64);
        }
        break;
        
        case 3:
        {
            CryptoPP::SHA384 hash;
            hash.Update(temp, length);
            hash.Final(digest);


            this->m_algorithm = digest[2] % 10;
            this->InitCrypto(this->m_algorithm, digest, 48);
        }
        break;

        case 4:
        {
            CryptoPP::Weak1::MD4 hash;
            hash.Update(temp, length);
            hash.Final(digest);


            this->m_algorithm = digest[3] % 10;
            this->InitCrypto(this->m_algorithm, digest, 16);
        }
        break;


        case 5:
        {
            CryptoPP::Weak1::MD5 hash;
            hash.Update(temp, length);
            hash.Final(digest);


            this->m_algorithm = digest[4] % 10;
            this->InitCrypto(this->m_algorithm, digest, 16);
        }
        break;


        case 6:
        {
            CryptoPP::RIPEMD160 hash;
            hash.Update(temp, length);
            hash.Final(digest);


            this->m_algorithm = digest[5] % 10;
            this->InitCrypto(this->m_algorithm, digest, 20);
        }
        break;


        case 7:
        {
            CryptoPP::RIPEMD320 hash;
            hash.Update(temp, length);
            hash.Final(digest);


            this->m_algorithm = digest[6] % 10;
            this->InitCrypto(this->m_algorithm, digest, 40);
        }
        break;


        case 8:
        {
            CryptoPP::RIPEMD128 hash;
            hash.Update(temp, length);
            hash.Final(digest);


            this->m_algorithm = digest[7] % 10;
            this->InitCrypto(this->m_algorithm, digest, 16);
        }
        break;


        case 9:
        {
            CryptoPP::RIPEMD256 hash;
            hash.Update(temp, length);
            hash.Final(digest);


            this->m_algorithm = digest[8] % 10;
            this->InitCrypto(this->m_algorithm, digest, 32);
        }
        break;


        default:
        {
            CryptoPP::Weak1::MD5 hash;
            hash.Update(temp, length);
            hash.Final(digest);


            this->m_algorithm = digest[9] % 10;
            this->InitCrypto(this->m_algorithm, digest, 16);
        }
        break;
    }



    return true;
}


bool CCryptoModulus::InitCrypto(unsigned int algorithm, unsigned char *key, unsigned int keyLength)
{
    if (this->m_cipher != NULL)
    {
        delete this->m_cipher;
        this->m_cipher = NULL;
    }

    switch (algorithm % 10)
    {
        case 1:
            this->m_cipher = new ConcreteCipher < CryptoPP::ThreeWay, 2048 > ;
            break;
        case 2:
            this->m_cipher = new ConcreteCipher < CryptoPP::CAST128, 2048 > ;
            break;
        case 4:
            this->m_cipher = new ConcreteCipher < CryptoPP::RC5, 2048 > ;
            break;
        case 5:
            this->m_cipher = new ConcreteCipher < CryptoPP::RC6, 2048 > ;
            break;
        case 6:
            this->m_cipher = new ConcreteCipher < CryptoPP::MARS, 2048 > ;
            break;
        case 7:
            this->m_cipher = new ConcreteCipher < CryptoPP::IDEA, 2048 > ;
            break;
        case 8:
            this->m_cipher = new ConcreteCipher < CryptoPP::GOST, 2048 > ;
            break;
        case 0:
            this->m_cipher = new ConcreteCipher < CryptoPP::TEA, 2048 > ;
            break;
        case 9:        
            {
                switch (key[0] % 8)
                {
                    case 0:
                        this->m_cipher = new ConcreteCipher < CryptoPP::Twofish, 8 > ;
                        break;
                    case 1:
                        this->m_cipher = new ConcreteCipher < CryptoPP::DES, 8 > ;
                        break;
                    case 2:
                        this->m_cipher = new ConcreteCipher < CryptoPP::DES_EDE2, 8 > ;
                        break;
                    case 3:
                        this->m_cipher = new ConcreteCipher < CryptoPP::DES_EDE3, 32 > ;
                        break;
                    case 4:
                        this->m_cipher = new ConcreteCipher < CryptoPP::DES_XEX3, 8 > ;
                        break;
                    case 5:
                        this->m_cipher = new ConcreteCipher < CryptoPP::SKIPJACK, 32 > ;
                        break;
                    case 6:
                        this->m_cipher = new ConcreteCipher < CryptoPP::SAFER_K, 8 > ;
                        break;
                    case 7:
                        this->m_cipher = new ConcreteCipher < CryptoPP::SAFER_SK, 32 > ;
                        break;
                    default:
                        this->m_cipher = new ConcreteCipher < CryptoPP::Twofish, 8 >;
                        break;
                }
            }
            break;
        default:
            this->m_cipher = new ConcreteCipher < CryptoPP::GOST, 2048 > ;
            break;
    }


    this->m_cipher->Init(key, keyLength);
    return true;
}


int CCryptoModulus::Encrypt(void *lpTarget, void *lpSource, int iSize)
{
    if (this->m_cipher)
    {
        return this->m_cipher->Encrypt((const unsigned char *)lpSource, iSize, (unsigned char *)lpTarget);
    }


    return -1;
}


int CCryptoModulus::Decrypt(void *lpTarget, void *lpSource, int iSize)
{
    if (this->m_cipher)
    {
        return this->m_cipher->Decrypt((const unsigned char *)lpSource, iSize, (unsigned char *)lpTarget);
    }

    return -1;
}


bool CCryptoModulus::LoadEncryptionKey(char *lpszFileName)
{
    return false;
}


bool CCryptoModulus::LoadDecryptionKey(char *lpszFileName)
{
    return false;
}


CSessionCryptor g_CryptoSessionCS;
CSessionCryptor g_CryptoSessionSC;


CSessionCryptor::CSessionCryptor()
{
	this->m_bEnableEncryptChange = false;
}


CSessionCryptor::~CSessionCryptor()
{
    this->cleanup();
}

void CSessionCryptor::InitKey(std:: string key)
{
	//if(Config.svr.m_season8ex == 1)
	{
		//this->m_defaultData = "98135003732920532106543021784355";
		this->m_defaultData = key;// "98132003732920532106543021784355";
	}
	//else // EX702
	//{
		//this->m_defaultData = "33234567890123456789012345678912";
	//}

	//OutputDebugString(this->m_defaultData.c_str());
}


bool CSessionCryptor::Open(int index)
{
    //std::map<int, CSessionCryptor::Cryptor *>::iterator i = this->m_cryptors.find(index);

	this->Close(index);

    /*if (i != this->m_cryptors.end())
    {
        this->m_cryptors.erase(i);
    }*/


    CSessionCryptor::Cryptor * cryptor = new CSessionCryptor::Cryptor;
    cryptor->crypto.InitCrypto((unsigned char *)this->m_defaultData.c_str(), this->m_defaultData.length());

    this->m_cryptors.insert(std::pair<int, CSessionCryptor::Cryptor *>(index, cryptor));
    return true;
}


void CSessionCryptor::ForceChange(int index, void *data, int iSize)
{
    std::map<int, CSessionCryptor::Cryptor *>::iterator i = this->m_cryptors.find(index);


    if (i == this->m_cryptors.end())
    {
        return;
    }


    i->second->crypto.InitCrypto((unsigned char *)data, iSize);
}


int CSessionCryptor::Encrypt(int index, void *lpTarget, void *lpSource, int iSize)
{
    std::map<int, CSessionCryptor::Cryptor *>::iterator i = this->m_cryptors.find(index);


    if (i == this->m_cryptors.end())
    {
        return -1;
    }


    CSessionCryptor::Cryptor *cryptor = i->second;
    int nsize = cryptor->crypto.Encrypt(lpTarget, lpSource, iSize);

    if (lpTarget)
    {
        if (cryptor->encCount == cryptor->crypto.GetMaxRunCount())
        {

            this->updateData(cryptor, (unsigned char *)lpSource, iSize);

			int oldalgo = this->GetAlgorithm(index);

            this->changeAlgorithm(cryptor);

            cryptor->encCount = 1;
        }
        else
        {
            this->updateData(cryptor, (unsigned char *)lpSource, iSize);

			if (this->m_bEnableEncryptChange == true)
			{
				cryptor->encCount++;
			}
        }
    }


    return nsize;
}


int CSessionCryptor::Decrypt(int index, void *lpTarget, void *lpSource, int iSize)
{
    std::map<int, CSessionCryptor::Cryptor *>::iterator i = this->m_cryptors.find(index);


    if (i == this->m_cryptors.end())
    {
        return -1;
    }


    CSessionCryptor::Cryptor *cryptor = i->second;
    int nsize = cryptor->crypto.Decrypt(lpTarget, lpSource, iSize);


    if (lpTarget)
    {
        if (cryptor->decCount == cryptor->crypto.GetMaxRunCount())
        {
            this->updateData(cryptor, (unsigned char *)lpTarget, nsize);

			int oldalgo = this->GetAlgorithm(index);

            this->changeAlgorithm(cryptor);

            cryptor->decCount = 1;
        }
        else
        {
            this->updateData(cryptor, (unsigned char *)lpTarget, nsize);

			if (this->m_bEnableEncryptChange == true)
			{
				cryptor->decCount++;
			}	
        }
    }


    return nsize;
}


void CSessionCryptor::Close(int index)
{
    std::map<int, CSessionCryptor::Cryptor *>::iterator i = this->m_cryptors.find(index);


    if (i == this->m_cryptors.end())
    {
        return;
    }


    if (i->second != NULL)
    {
        delete i->second;
    }


    this->m_cryptors.erase(i);
}


int CSessionCryptor::GetEncryptCount(int index)
{
    std::map<int, CSessionCryptor::Cryptor *>::iterator i = this->m_cryptors.find(index);


    if (i == this->m_cryptors.end())
    {
        return 0;
    }


    return i->second->encCount;
}


int CSessionCryptor::GetDecryptCount(int index)
{
    std::map<int, CSessionCryptor::Cryptor *>::iterator i = this->m_cryptors.find(index);


    if (i == this->m_cryptors.end())
    {
        return 0;
    }


    return i->second->decCount;
}


int CSessionCryptor::GetAlgorithm(int index)
{
    std::map<int, CSessionCryptor::Cryptor *>::iterator i = this->m_cryptors.find(index);


    if (i == this->m_cryptors.end())
    {
        return 0;
    }


    return i->second->crypto.GetAlgorithm();
}


void CSessionCryptor::updateData(CSessionCryptor::Cryptor *cryptor, unsigned char *buf, int size)
{
    if (size)
    {
        if (size <= 8)
        {
            cryptor->data[buf[0] & 0x7F] = buf[0];
        }


        else
        {
            cryptor->data[buf[0] & 0x7F] = buf[0];
            cryptor->data[buf[1] & 0x7F] = buf[1];
            cryptor->data[buf[2] & 0x7F] = buf[2];
            cryptor->data[buf[3] & 0x7F] = buf[3];
            cryptor->data[buf[4] & 0x7F] = buf[4];
            cryptor->data[buf[5] & 0x7F] = buf[5];
            cryptor->data[buf[6] & 0x7F] = buf[6];
            cryptor->data[buf[7] & 0x7F] = buf[7];
        }
    }
}


void CSessionCryptor::changeAlgorithm(CSessionCryptor::Cryptor *cryptor)
{
    cryptor->crypto.InitCrypto(cryptor->data, 128);
}


void CSessionCryptor::cleanup()
{
    std::map<int, CSessionCryptor::Cryptor *>::iterator i = this->m_cryptors.begin();


    while (i != this->m_cryptors.end())
    {
        if (i->second != NULL)
        {
            delete i->second;
        }


        this->m_cryptors.erase(i);
        i++;
    }
}