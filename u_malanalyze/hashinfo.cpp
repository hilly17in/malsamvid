#include "stdafx.h"

void hashinfo::SetFilePath(const WCHAR *arg)
{
	StringCbCopy(FilePath, MAX_IMAGE_PATH, arg);

	if(FilePath[0]=='\\')
		convertwithdrive(FilePath, wcslen(arg), dl_FilePath);
	else
		StringCbCopy(dl_FilePath, MAX_IMAGE_PATH, FilePath);
}	

void hashinfo::GetFilePath(WCHAR *param)
{
	StringCbCopy(param, MAX_IMAGE_PATH, dl_FilePath);
}

void hashinfo::GetMD5(BYTE *param, int print)
{
	strcpy((char *)param, (char *)md5);

	if(print == 1)
	{
		int index=0;
		while(index < MD5LEN)
			printf("%02X", md5[index++]);
	}
}

void hashinfo::GetMD5(WCHAR *param, int print)
{
	StringCbCopy(param, WMD5LEN*sizeof(WCHAR), wmd5);

	if(print == 1)
		printf("\n hash = %ws \n", wmd5);
}

void hashinfo::CalcAllHashes()
{
	CalcMD5();
	//CalcSHA1();
	//CalcSHA256();
	//CalcSHA512();
}

DWORD hashinfo::CalcMD5()
{
    DWORD dwStatus = 0;
    BOOL bResult = FALSE;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    HANDLE hFile = NULL;
    BYTE rgbFile[BUFSIZE];
    DWORD cbRead = 0;
    DWORD cbHash = 0;
    CHAR rgbDigits[] = "0123456789abcdef";
   
    // Logic to check usage goes here.
    hFile = CreateFile(dl_FilePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwStatus = GetLastError();
        printf("Error opening file %ws\nError: %d\n", dl_FilePath, 
            dwStatus); 
        return dwStatus;
    }

    // Get handle to the crypto provider
    if (!CryptAcquireContext(&hProv,
        NULL,
        NULL,
        PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT))
    {
        dwStatus = GetLastError();
        printf("CryptAcquireContext failed: %d\n", dwStatus); 
        CloseHandle(hFile);
        return dwStatus;
    }

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        dwStatus = GetLastError();
        printf("CryptAcquireContext failed: %d\n", dwStatus); 
        CloseHandle(hFile);
        CryptReleaseContext(hProv, 0);
        return dwStatus;
    }

    while (bResult = ReadFile(hFile, rgbFile, BUFSIZE, 
        &cbRead, NULL))
    {
        if (0 == cbRead)
        {
            break;
        }

        if (!CryptHashData(hHash, rgbFile, cbRead, 0))
        {
            dwStatus = GetLastError();
            printf("CryptHashData failed: %d\n", dwStatus); 
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            CloseHandle(hFile);
            return dwStatus;
        }
    }

    if (!bResult)
    {
        dwStatus = GetLastError();
        printf("ReadFile failed: %d\n", dwStatus); 
        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);
        CloseHandle(hFile);
        return dwStatus;
    }

    cbHash = MD5LEN;
    if (CryptGetHashParam(hHash, HP_HASHVAL, md5, &cbHash, 0))
    {
        //printf("MD5 hash of file %s is: ", dl_FilePath);
        //for (DWORD i = 0; i < cbHash; i++)
        //{
        //    printf("%c%c", rgbDigits[md5[i] >> 4],
        //        rgbDigits[md5[i] & 0xf]);
        //}
        //printf("\n");
    }
    else
    {
        dwStatus = GetLastError();
        printf("CryptGetHashParam failed: %d\n", dwStatus); 
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);

	// convert md5 to wchar string
	for( int i=0; i<16; i++ )
		StringCbPrintf( &wmd5[i * 2], 6, L"%02x", md5[i] );


	return dwStatus;
}
//
//void hashinfo::CalcSHA1()
//{
//
//}
//
//void hashinfo::CalcSHA256()
//{
//
//}
//
//void hashinfo::CalcSHA512()
//{
//
//}
