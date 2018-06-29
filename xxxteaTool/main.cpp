#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xxtea.h"
#include "StatDir.h"
#include <map>
#include<windows.h>

//void main()
//{
//	//获取目录名  
//	char buf[256];
//	printf("请输入要统计的目录名:");
//	gets(buf);
//
//	//构造类对象  
//	CStatDir statdir;
//
//	//设置要遍历的目录  
//	if (!statdir.SetInitDir(buf))
//	{
//		puts("目录不存在。");
//		return;
//	}
//
//	//开始遍历  
//
//	vector<string>file_vec = statdir.BeginBrowseFilenames("*.*");
//	for (vector<string>::const_iterator it = file_vec.begin(); it < file_vec.end(); ++it)
//		std::cout << *it << std::endl;
//
//	printf("文件总数: %d\n", file_vec.size());
//	system("pause");
//}

// 加密文件
void encrypt(const char *szKey, const char *szSign, string szSrcFile, string szOutFile)
{
	FILE *fp;
	errno_t err;
	if ((err = fopen_s(&fp, szSrcFile.c_str(), "rb")) != 0)
	{
		perror("can't open the input file");
		return;
	}
	fseek(fp, 0L, SEEK_END);
	unsigned long size = ftell(fp);
	rewind(fp);
	unsigned char *buf = (unsigned char *)malloc(size);
	fread(buf, size, 1, fp);
	fclose(fp);

	xxtea_long nKeylen = strlen(szKey);
	xxtea_long nSignLen = strlen(szSign);
	xxtea_long nRetlen = 0;
	unsigned char *data = xxtea_encrypt(buf, size, (unsigned char *)szKey, nKeylen, &nRetlen);
	if (data == NULL)
	{
		printf("%s encrypt fail\n", szSrcFile.c_str());
		return;
	}
	
	if ((err = fopen_s(&fp, szOutFile.c_str(), "wb+")) != 0)
	{
		perror("can't open the output file");
		return;
	}

	unsigned char *pNewData = (unsigned char *)malloc(nRetlen + nSignLen);
	memcpy(pNewData, szSign, nSignLen);
	memcpy(pNewData + nSignLen, data, nRetlen);
	fwrite(pNewData, nRetlen + nSignLen, 1, fp);

	fclose(fp);
	free(data);
	free(pNewData);

	printf("%s encrypt successful\n", szSrcFile.c_str());
}

// 解密文件
void decrypt(const char *szKey, const char *szSign, string szSrcFile, string szOutFile)
{
	FILE *fp;
	errno_t err;
	if ((err = fopen_s(&fp, szSrcFile.c_str(), "rb")) != 0)
	{
		perror("can't open the input file");
		return;
	}
	fseek(fp, 0L, SEEK_END);
	unsigned long size = ftell(fp);
	rewind(fp);
	unsigned char *buf = (unsigned char *)malloc(size);
	fread(buf, size, 1, fp);
	fclose(fp);

	xxtea_long nKeylen = strlen(szKey);
	xxtea_long nSignLen = strlen(szSign);
	xxtea_long nRetlen = 0;
	// 如果加密的数据最开始几个字节如果与签名不相符，则不是加密的数据
	if (strncmp((const char*)buf, (const char*)szSign, nSignLen) != 0)
	{
		return;
	}
	
	unsigned char *data = xxtea_decrypt(buf + nSignLen, size - nSignLen, (unsigned char *)szKey, nKeylen, &nRetlen);
	if (data == NULL)
	{
		printf("%s decrypt fail\n", szSrcFile.c_str());
		return;
	}
	string strOld = ".luac";
	string strNew = ".lua";
	string strEnd = szOutFile.substr(szOutFile.length() - strOld.length(), strOld.length());
	string str = "";
	if (strEnd == strOld)
	{
		str = szOutFile.substr(0, szOutFile.length() - strOld.length()) + strNew;
	}
	if ((err = fopen_s(&fp, str.c_str(), "wb+")) != 0)
	{
		perror("can't open the output file");
		return;
	}
	fwrite(data, nRetlen, 1, fp);
	fclose(fp);
	free(data);
	printf("%s decrypt successful\n", szSrcFile.c_str());
}

int main(int argc, char *argv[])
{
	char *szKey = NULL;
	char *szSign = NULL;
	char *szSrcFile = NULL;
	char *szOutFile = NULL;
	bool bEncrypt = false;
	char *szExclude = NULL;
	
	if (argc < 5)
	{
		printf("usage:lua_decrypt szSrcFile szOutFile szSign szKey\n");
		return -1;
	}
	else if (argc == 5)
	{
		szSrcFile = argv[1];
		szOutFile = argv[2];
		szSign = argv[3];
		szKey = argv[4];
	}
	else
	{
		szSrcFile = argv[1];
		szOutFile = argv[2];
		szSign = argv[3];
		szKey = argv[4];
		bEncrypt = (bool)atoi(argv[5]);
		szExclude = argv[6];
	}

	//构造类对象  
	CStatDir statdir;

	char szOutDir[_MAX_PATH] = { 0 };
	//用当前目录初始化  
	_getcwd(szOutDir, _MAX_PATH);
	//先把dir转换为绝对路径
	if (_fullpath(szOutDir, szOutFile, _MAX_PATH) == NULL)
	{
		return false;
	}
	//如果目录的最后一个字母不是'\',则在最后加上一个'\'  
	int nOutLen = strlen(szOutDir);
	if (szOutDir[nOutLen - 1] != '\\')
	{
		strcat_s(szOutDir, _MAX_PATH, "\\");
	}

	char szInDir[_MAX_PATH] = { 0 };
	//用当前目录初始化  
	_getcwd(szInDir, _MAX_PATH);
	//先把dir转换为绝对路径
	if (_fullpath(szInDir, szSrcFile, _MAX_PATH) == NULL)
	{
		return false;
	}
	//如果目录的最后一个字母不是'\',则在最后加上一个'\'  
	int nInLen = strlen(szInDir);
	if (szInDir[nInLen - 1] != '\\')
	{
		strcat_s(szInDir, _MAX_PATH, "\\");
	}

	printf("%d , szInDir=%s,szOutDir=%s,bEncrypt=%d", argc, szInDir, szOutDir, bEncrypt);
	//设置要遍历的目录  
	if (!statdir.SetInitDir(szInDir))
	{
		puts("目录不存在。");
		return -1;
	}
	printf("%d , szInDir=%s,szOutDir=%s,bEncrypt=%d", argc, szInDir, szOutDir, bEncrypt);

	// 删除目录
	statdir.DeleteDirectory(szOutDir);
	// 创建目录
	statdir.CreateDirectory(szOutDir);

	std::map<string, bool> vExcludeFile;
	string szExcludeFile(szExclude);
	int nLastPos = 0;
	int nPos = szExcludeFile.find_first_of(';');
	while (nPos != string::npos)
	{
		string szFileType(szExcludeFile, nLastPos, nPos - nLastPos);
		vExcludeFile[szFileType] = true;

		nLastPos = nPos + 1;
		nPos = szExcludeFile.find_first_of(";", nPos + 1);
	}

	auto vFile = statdir.BeginBrowseFilenames("*.*");
	for (auto it = vFile.begin(); it < vFile.end(); ++it)
	{
		int pos = it->find_last_of('.');
		//string szSrcPath(*it, 0, pos);
		string szSrcFileName(it->substr(pos));

		// 输出目录
		string szOutPath(szOutDir);
		szOutPath.append(it->substr(strlen(szInDir)));

		statdir.CreateDirectory(szOutPath.c_str());

		// 排除文件
		if (vExcludeFile.find(szSrcFileName) != vExcludeFile.end())
		{
			CopyFileA(it->c_str(), szOutPath.c_str(), false);
			continue;
		}
		
		if (bEncrypt)
		{
			// 加密
			encrypt(szKey, szSign, *it, szOutPath.c_str());
		}
		else
		{
			// 解密
			decrypt(szKey, szSign, *it, szOutPath.c_str());
		}
	}
	
	return 0;
}
