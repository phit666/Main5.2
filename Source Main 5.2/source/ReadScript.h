#ifdef _MU_SDL_FILE
#include "mu_file.h"

static enum SMDToken
{
	NAME,
	NUMBER,
	END,
	COMMAND = '#',
	LP = '{',
	RP = '}',
	COMMA = ',',
	SEMICOLON = ';',
	SMD_ERROR
};

static MU_FILE* SMDFile;
static float    TokenNumber;
static char     TokenString[256];
static SMDToken CurrentToken;

inline int MU_fgetc(MU_FILE* fp)
{
	if (!fp)
		return EOF;

	unsigned char ch;

	if (SDL_RWread(fp, &ch, 1, 1) != 1)
		return EOF;

	return (int)ch;
}

inline int MU_getc(MU_FILE* fp)
{
	return MU_fgetc(fp);
}

inline int MU_ungetc(int c, MU_FILE* fp)
{
	if (!fp || c == EOF)
		return EOF;

	Sint64 pos = SDL_RWtell(fp);

	if (pos <= 0)
		return EOF;

	if (SDL_RWseek(fp, pos - 1, RW_SEEK_SET) < 0)
		return EOF;

	return c;
}

static SMDToken GetToken()
{
	int ch;
	TokenString[0] = '\0';

	do
	{
		ch = MU_fgetc(SMDFile);
		if (ch == EOF)
			return END;

		if (ch == '/' && (ch = MU_fgetc(SMDFile)) == '/')
		{
			while ((ch = MU_fgetc(SMDFile)) != EOF && ch != '\n')
			{
			}
		}
	} while (isspace((unsigned char)ch));

	char* p;
	char TempString[100];

	switch (ch)
	{
	case '#':
		return CurrentToken = COMMAND;
	case ';':
		return CurrentToken = SEMICOLON;
	case ',':
		return CurrentToken = COMMA;
	case '{':
		return CurrentToken = LP;
	case '}':
		return CurrentToken = RP;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '.': case '-':
		MU_ungetc(ch, SMDFile);

		p = TempString;

		while ((ch = MU_getc(SMDFile)) != EOF &&
			(ch == '.' || isdigit((unsigned char)ch) || ch == '-'))
		{
			*p++ = (char)ch;
		}

		if (ch != EOF)
			MU_ungetc(ch, SMDFile);

		*p = 0;

		TokenNumber = (float)atof(TempString);

		return CurrentToken = NUMBER;

	case '"':
		p = TokenString;

		while ((ch = MU_getc(SMDFile)) != EOF && ch != '"')
		{
			*p++ = (char)ch;
		}

		if (ch != '"')
			MU_ungetc(ch, SMDFile);

		*p = 0;

		return CurrentToken = NAME;

	default:
		if (isalpha((unsigned char)ch))
		{
			p = TokenString;
			*p++ = (char)ch;

			while ((ch = MU_getc(SMDFile)) != EOF &&
				(ch == '.' || ch == '_' || isalnum((unsigned char)ch)))
			{
				*p++ = (char)ch;
			}

			if (ch != EOF)
				MU_ungetc(ch, SMDFile);

			*p = 0;

			return CurrentToken = NAME;
		}

		return CurrentToken = SMD_ERROR;
	}
}
#else
static enum SMDToken 
{
	NAME, 
	NUMBER, 
	END, 
	COMMAND = '#',
	LP = '{',
	RP = '}',
	COMMA = ',',
	SEMICOLON = ';',
	SMD_ERROR
};

static FILE     *SMDFile;
static float    TokenNumber;
static char     TokenString[256];
static SMDToken CurrentToken;

static SMDToken GetToken()
{
	char ch;
	TokenString[0] = '\0';
	do
	{
		if ( (ch =(char) fgetc(SMDFile)) == EOF) return END;
		if (ch=='/' && (ch =(char) fgetc(SMDFile) )=='/')	
		{
			while( (ch = (char) fgetc( SMDFile)) != '\n' );
		}
	} while(  isspace(ch) );
	
	char *p, TempString[100];
	switch(ch)
	{	
	case '#':
		return CurrentToken = COMMAND;
	case ';':
		return CurrentToken = SEMICOLON;
	case ',':
		return CurrentToken = COMMA;
	case '{':
		return CurrentToken = LP;
	case '}':
		return CurrentToken = RP;
	case '0':	case '1':	case '2':	case '3':	case '4':
	case '5':	case '6':	case '7':	case '8':	case '9':
	case '.':	case '-':
		ungetc(ch,SMDFile);
		p = TempString;
		while ( (  (ch = (char)getc(SMDFile) ) !=EOF) && (ch=='.' || isdigit(ch) || ch=='-') )
			*p++ = ch;
		*p = 0;
		TokenNumber = (float)atof(TempString);
		//			sscanf(TempString," %f ",&TokenNumber);
		return CurrentToken = NUMBER;
	case '"':
		p = TokenString;
		while ( (  (ch = (char)getc(SMDFile) ) !=EOF) && (ch!='"'))// || isalnum(ch)) )
			*p++ = ch;
		if (ch!='"')
			ungetc(ch,SMDFile);
		*p = 0;
		return CurrentToken = NAME;
	default:
		if (isalpha(ch))	
		{
			p = TokenString;
			*p++ = ch;
			while ( (  (ch = (char)getc(SMDFile) ) !=EOF) && (ch=='.' || ch=='_' || isalnum(ch)) )
				*p++ = ch;
			ungetc(ch,SMDFile);
			*p = 0;
			return CurrentToken = NAME;
		}
		return CurrentToken = SMD_ERROR;
	}
}
#endif