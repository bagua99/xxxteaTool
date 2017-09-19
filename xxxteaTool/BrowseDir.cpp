#include "BrowseDir.h"

CBrowseDir::CBrowseDir()
{
	//�õ�ǰĿ¼��ʼ��m_szInitDir  
	_getcwd(m_szInitDir, _MAX_PATH);

	//���Ŀ¼�����һ����ĸ����'\',����������һ��'\'  
	int len = strlen(m_szInitDir);
	if (m_szInitDir[len - 1] != '\\')
		strcat_s(m_szInitDir, _MAX_PATH, "\\");
}

bool CBrowseDir::SetInitDir(const char *dir)
{
	//�Ȱ�dirת��Ϊ����·��  
	if (_fullpath(m_szInitDir, dir, _MAX_PATH) == NULL)
		return false;

	//�ж�Ŀ¼�Ƿ����  
	if (_chdir(m_szInitDir) != 0)
		return false;

	//���Ŀ¼�����һ����ĸ����'\',����������һ��'\'  
	int len = strlen(m_szInitDir);
	if (m_szInitDir[len - 1] != '\\')
		strcat_s(m_szInitDir, _MAX_PATH, "\\");

	return true;
}

vector<string> CBrowseDir::BeginBrowseFilenames(const char *filespec)
{
	ProcessDir(m_szInitDir, NULL);
	return GetDirFilenames(m_szInitDir, filespec);
}

bool CBrowseDir::BeginBrowse(const char *filespec)
{
	ProcessDir(m_szInitDir, NULL);
	return BrowseDir(m_szInitDir, filespec);
}

bool CBrowseDir::BrowseDir(const char *dir, const char *filespec)
{
	_chdir(dir);

	//���Ȳ���dir�з���Ҫ����ļ�  
	long hFile;
	_finddata_t fileinfo;
	if ((hFile = _findfirst(filespec, &fileinfo)) != -1)
	{
		do
		{
			//����ǲ���Ŀ¼  
			//�������,����д���  
			if (!(fileinfo.attrib & _A_SUBDIR))
			{
				char filename[_MAX_PATH];
				strcpy_s(filename, _MAX_PATH, dir);
				strcat_s(filename, _MAX_PATH, fileinfo.name);
				cout << filename << endl;
				if (!ProcessFile(filename))
					return false;
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	//����dir�е���Ŀ¼  
	//��Ϊ�ڴ���dir�е��ļ�ʱ���������ProcessFile�п��ܸı���  
	//��ǰĿ¼����˻�Ҫ�������õ�ǰĿ¼Ϊdir��  
	//ִ�й�_findfirst�󣬿���ϵͳ��¼���������Ϣ����˸ı�Ŀ¼  
	//��_findnextû��Ӱ�졣  
	_chdir(dir);
	if ((hFile = _findfirst("*.*", &fileinfo)) != -1)
	{
		do
		{
			//����ǲ���Ŀ¼  
			//�����,�ټ���ǲ��� . �� ..   
			//�������,���е���  
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp
					(fileinfo.name, "..") != 0)
				{
					char subdir[_MAX_PATH];
					strcpy_s(subdir, _MAX_PATH, dir);
					strcat_s(subdir, _MAX_PATH, fileinfo.name);
					strcat_s(subdir, _MAX_PATH, "\\");
					ProcessDir(subdir, dir);
					if (!BrowseDir(subdir, filespec))
						return false;
				}
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return true;
}

vector<string> CBrowseDir::GetDirFilenames(const char *dir, const char *filespec)
{
	_chdir(dir);
	vector<string> filename_vector;
	filename_vector.clear();

	//���Ȳ���dir�з���Ҫ����ļ�  
	long hFile;
	_finddata_t fileinfo;
	if ((hFile = _findfirst(filespec, &fileinfo)) != -1)
	{
		do
		{
			//����ǲ���Ŀ¼  
			//�������,����д���  
			if (!(fileinfo.attrib & _A_SUBDIR))
			{
				char filename[_MAX_PATH];
				strcpy_s(filename, _MAX_PATH, dir);
				strcat_s(filename, _MAX_PATH, fileinfo.name);
				filename_vector.push_back(filename);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	//����dir�е���Ŀ¼  
	//��Ϊ�ڴ���dir�е��ļ�ʱ���������ProcessFile�п��ܸı���  
	//��ǰĿ¼����˻�Ҫ�������õ�ǰĿ¼Ϊdir��  
	//ִ�й�_findfirst�󣬿���ϵͳ��¼���������Ϣ����˸ı�Ŀ¼  
	//��_findnextû��Ӱ�졣  
	_chdir(dir);
	if ((hFile = _findfirst("*.*", &fileinfo)) != -1)
	{
		do
		{
			//����ǲ���Ŀ¼  
			//�����,�ټ���ǲ��� . �� ..   
			//�������,���е���  
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp
					(fileinfo.name, "..") != 0)
				{
					char subdir[_MAX_PATH] = {0};
					strcpy_s(subdir, _MAX_PATH, dir);
					strcat_s(subdir, _MAX_PATH, fileinfo.name);
					strcat_s(subdir, _MAX_PATH, "\\");
					ProcessDir(subdir, dir);
					vector<string>tmp = GetDirFilenames(subdir, filespec);
					for (vector<string>::iterator it = tmp.begin(); it < tmp.end(); it++)
					{
						filename_vector.push_back(*it);
					}
				}
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return filename_vector;
}

bool CBrowseDir::IsDirectory(const char *pDir)
{
	char szCurPath[MAX_PATH] = {0};
	sprintf_s(szCurPath, MAX_PATH, "%s//*", pDir);
	WIN32_FIND_DATAA FindFileData;
	ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));

	HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		return false; /** ��������ҵ���һ���ļ�����ôû��Ŀ¼ */
	}
	else
	{
		FindClose(hFile);
		return true;
	}
}

BOOL CBrowseDir::DeleteDirectory(const char *DirName)
{
	char szCurPath[MAX_PATH] = {0};					//���ڶ���������ʽ
	_snprintf_s(szCurPath, MAX_PATH, "%s//*.*", DirName);    //ƥ���ʽΪ*.*,����Ŀ¼�µ������ļ�
	WIN32_FIND_DATAA FindFileData;
	ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));
	HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData);
	BOOL IsFinded = true;
	while (IsFinded)
	{
		IsFinded = FindNextFileA(hFile, &FindFileData);    //�ݹ������������ļ�
		if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, "..")) //�������"." ".."Ŀ¼
		{
			std::string strFileName = "";
			strFileName = strFileName + DirName + "//" + FindFileData.cFileName;
			std::string strTemp;
			strTemp = strFileName;
			if (IsDirectory(strFileName.c_str())) //�����Ŀ¼����ݹ�ص���
			{
				DeleteDirectory(strTemp.c_str());
			}
			else
			{
				DeleteFileA(strTemp.c_str());
			}
		}
	}
	FindClose(hFile);

	BOOL bRet = RemoveDirectoryA(DirName);
	if (bRet == 0) //ɾ��Ŀ¼
	{
		return FALSE;
	}
	return TRUE;
}

// ����Ŀ¼
void CBrowseDir::CreateDirectory(const char *filename)
{
	string szFile(filename);
	int nPos = szFile.find_first_of("\\");
	while (nPos != string::npos)
	{
		string szSrcPath(szFile, 0, nPos);
		if (_access(szSrcPath.c_str(), 0) == -1)
		{
			_mkdir(szSrcPath.c_str());
		}

		nPos = szFile.find_first_of("\\", nPos + 1);
	}
}

bool CBrowseDir::ProcessFile(const char *filename)
{
	return true;
}

void CBrowseDir::ProcessDir(const char *currentdir, const char *parentdir)
{
}