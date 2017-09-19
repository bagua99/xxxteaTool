#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xxtea.h"
#include "StatDir.h"
#include <map>
#include<windows.h>

//void main()
//{
//	//��ȡĿ¼��  
//	char buf[256];
//	printf("������Ҫͳ�Ƶ�Ŀ¼��:");
//	gets(buf);
//
//	//���������  
//	CStatDir statdir;
//
//	//����Ҫ������Ŀ¼  
//	if (!statdir.SetInitDir(buf))
//	{
//		puts("Ŀ¼�����ڡ�");
//		return;
//	}
//
//	//��ʼ����  
//
//	vector<string>file_vec = statdir.BeginBrowseFilenames("*.*");
//	for (vector<string>::const_iterator it = file_vec.begin(); it < file_vec.end(); ++it)
//		std::cout << *it << std::endl;
//
//	printf("�ļ�����: %d\n", file_vec.size());
//	system("pause");
//}

// �����ļ�
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

// �����ļ�
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
	// ������ܵ������ʼ�����ֽ������ǩ������������Ǽ��ܵ�����
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
	if ((err = fopen_s(&fp, szOutFile.c_str(), "wb+")) != 0)
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

	//���������  
	CStatDir statdir;

	char szOutDir[_MAX_PATH] = { 0 };
	//�õ�ǰĿ¼��ʼ��  
	_getcwd(szOutDir, _MAX_PATH);
	//�Ȱ�dirת��Ϊ����·��
	if (_fullpath(szOutDir, szOutFile, _MAX_PATH) == NULL)
	{
		return false;
	}
	//���Ŀ¼�����һ����ĸ����'\',����������һ��'\'  
	int nOutLen = strlen(szOutDir);
	if (szOutDir[nOutLen - 1] != '\\')
	{
		strcat_s(szOutDir, _MAX_PATH, "\\");
	}

	char szInDir[_MAX_PATH] = { 0 };
	//�õ�ǰĿ¼��ʼ��  
	_getcwd(szInDir, _MAX_PATH);
	//�Ȱ�dirת��Ϊ����·��
	if (_fullpath(szInDir, szSrcFile, _MAX_PATH) == NULL)
	{
		return false;
	}
	//���Ŀ¼�����һ����ĸ����'\',����������һ��'\'  
	int nInLen = strlen(szInDir);
	if (szInDir[nInLen - 1] != '\\')
	{
		strcat_s(szInDir, _MAX_PATH, "\\");
	}

	printf("%d , szInDir=%s,szOutDir=%s,bEncrypt=%d", argc, szInDir, szOutDir, bEncrypt);
	//����Ҫ������Ŀ¼  
	if (!statdir.SetInitDir(szInDir))
	{
		puts("Ŀ¼�����ڡ�");
		return -1;
	}
	printf("%d , szInDir=%s,szOutDir=%s,bEncrypt=%d", argc, szInDir, szOutDir, bEncrypt);

	// ɾ��Ŀ¼
	statdir.DeleteDirectory(szOutDir);
	// ����Ŀ¼
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

		// ���Ŀ¼
		string szOutPath(szOutDir);
		szOutPath.append(it->substr(strlen(szInDir)));

		statdir.CreateDirectory(szOutPath.c_str());

		// �ų��ļ�
		if (vExcludeFile.find(szSrcFileName) != vExcludeFile.end())
		{
			CopyFileA(it->c_str(), szOutPath.c_str(), false);
			continue;
		}
		
		if (bEncrypt)
		{
			// ����
			encrypt(szKey, szSign, *it, szOutPath.c_str());
		}
		else
		{
			// ����
			decrypt(szKey, szSign, *it, szOutPath.c_str());
		}
	}
	
	return 0;
}