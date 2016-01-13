#pragma once

#define MD5LEN		16
#define SHA1LEN		20
#define SHA2LEN		32
#define SHA3LEN		64

#define WMD5LEN		33
class hashinfo
{
private:

	WCHAR	FilePath[MAX_IMAGE_PATH];
	WCHAR	dl_FilePath[MAX_IMAGE_PATH];
	
	BYTE	md5[MD5LEN];
	WCHAR	wmd5[WMD5LEN];
	//BYTE	sha1[SHA1LEN];
	//BYTE	sha2[SHA2LEN];
	//BYTE	sha3[SHA3LEN];

public:

	void SetFilePath(const WCHAR *);
	void GetFilePath(WCHAR *);
	//void SetMD5();
	void GetMD5(BYTE  *param, int print = 0);
	void GetMD5(WCHAR *param, int print = 0);
	void CalcAllHashes();
	DWORD CalcMD5();
	//void CalcSHA1();
	//void CalcSHA256();
	//void CalcSHA512();
};

