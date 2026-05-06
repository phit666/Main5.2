//////////////////////////////////////////////////////////////////////
// Simple Modulus ( Cryptography library ) ver 1.1
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLEMODULUS_H__5EC20CDC_CA87_4F9D_927B_24C8DD11ECB7__INCLUDED_)
#define AFX_SIMPLEMODULUS_H__5EC20CDC_CA87_4F9D_927B_24C8DD11ECB7__INCLUDED_

#pragma once

/*# define SIZE_ENCRYPTION_BLOCK	( 32)
# define SIZE_ENCRYPTION_KEY	( 16)
# define SIZE_ENCRYPTED_BLOCK	( 38)*/

/*# define SIZE_ENCRYPTION_BLOCK	( 16)
# define SIZE_ENCRYPTION_KEY	( 8)	// SIZE_ENCRYPTION_BLOCK
# define SIZE_ENCRYPTED_BLOCK	( 20)*/

# define SIZE_ENCRYPTION_BLOCK	( 8)
# define SIZE_ENCRYPTION_KEY	( 4)	// SIZE_ENCRYPTION_BLOCK
# define SIZE_ENCRYPTED_BLOCK	( 11)

#pragma pack(1)
struct ENCDEC_FILEHEADER
{
    short sFileHeader;
    int dwSize;
};
#pragma pack()

class CSimpleModulus  
{
public:
	CSimpleModulus();
	virtual ~CSimpleModulus();

	void Init( void);

protected:
	DWORD m_dwModulus[SIZE_ENCRYPTION_KEY];
	DWORD m_dwEncryptionKey[SIZE_ENCRYPTION_KEY];
	DWORD m_dwDecryptionKey[SIZE_ENCRYPTION_KEY];
	DWORD m_dwXORKey[SIZE_ENCRYPTION_KEY];

	static DWORD s_dwSaveLoadXOR[SIZE_ENCRYPTION_KEY];

public:
	int Encrypt( void *lpTarget, void *lpSource, int iSize);
	int Decrypt( void *lpTarget, void *lpSource, int iSize);

protected:
	int EncryptBlock( void *lpTarget, void *lpSource, int nSize);
	int DecryptBlock( void *lpTarget, void *lpSource);
	int AddBits( void *lpBuffer, int nNumBufferBits, void *lpBits, int nInitialBit, int nNumBits);
	void Shift( void *lpBuffer, int nByte, int nShift);
	int GetByteOfBit( int nBit);
public:
	BOOL SaveAllKey(LPSTR lpszFileName);
	BOOL LoadAllKey(LPSTR lpszFileName);
	BOOL SaveEncryptionKey(LPSTR lpszFileName);
	BOOL LoadEncryptionKey(LPSTR lpszFileName);
	BOOL SaveDecryptionKey(LPSTR lpszFileName);
	BOOL LoadDecryptionKey(LPSTR lpszFileName);
protected:
	BOOL SaveKey(LPSTR lpszFileName, WORD wFileHeader, BOOL bSaveModulus, BOOL bSaveEncKey, BOOL bSaveDecKey, BOOL bSaveXORKey);
	//BOOL SaveKey(LPSTR lpszFileName, unsigned short sID, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR);
	//BOOL LoadKey(LPSTR lpszFileName, unsigned short sID, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR);
	BOOL LoadKey(LPSTR lpszFileName,
		WORD wFileHeader,
		BOOL bLoadModulus,
		BOOL bLoadEncKey,
		BOOL bLoadDecKey,
		BOOL bLoadXORKey);
public:
	BOOL LoadKeyFromBuffer( BYTE *pbyBuffer, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR);
};


#pragma pack ( 1)
typedef struct
{
	unsigned short m_sID;
	unsigned int m_iSize;
} ChunkHeader;
#pragma pack ()

# define CHUNKID_ALLKEY		( 0x1111)
# define CHUNKID_ONEKEY		( 0x1112)


#endif // !defined(AFX_SIMPLEMODULUS_H__5EC20CDC_CA87_4F9D_927B_24C8DD11ECB7__INCLUDED_)
