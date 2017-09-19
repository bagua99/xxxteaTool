#include "BrowseDir.h"

CBrowseDir::CBrowseDir()
{
	//用当前目录初始化m_szInitDir  
	_getcwd(m_szInitDir, _MAX_PATH);

	//如果目录的最后一个字母不是'\',则在最后加上一个'\'  
	int len = strlen(m_szInitDir);
	if (m_szInitDir[len - 1] != '\\')
		strcat_s(m_szInitDir, _MAX_PATH, "\\");
}

bool CBrowseDir::SetInitDir(const char *dir)
{
	//先把dir转换为绝对路径  
	if (_fullpath(m_szInitDir, dir, _MAX_PATH) == NULL)
		return false;

	//判断目录是否存在  
	if (_chdir(m_szInitDir) != 0)
		return false;

	//如果目录的最后一个字母不是'\',则在最后加上一个'\'  
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

	//首先查找dir中符合要求的文件  
	long hFile;
	_finddata_t fileinfo;
	if ((hFile = _findfirst(filespec, &fileinfo)) != -1)
	{
		do
		{
			//检查是不是目录  
			//如果不是,则进行处理  
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
	//查找dir中的子目录  
	//因为在处理dir中的文件时，派生类的ProcessFile有可能改变了  
	//当前目录，因此还要重新设置当前目录为dir。  
	//执行过_findfirst后，可能系统记录下了相关信息，因此改变目录  
	//对_findnext没有影响。  
	_chdir(dir);
	if ((hFile = _findfirst("*.*", &fileinfo)) != -1)
	{
		do
		{
			//检查是不是目录  
			//如果是,再检查是不是 . 或 ..   
			//如果不是,进行迭代  
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

	//首先查找dir中符合要求的文件  
	long hFile;
	_finddata_t fileinfo;
	if ((hFile = _findfirst(filespec, &fileinfo)) != -1)
	{
		do
		{
			//检查是不是目录  
			//如果不是,则进行处理  
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
	//查找dir中的子目录  
	//因为在处理dir中的文件时，派生类的ProcessFile有可能改变了  
	//当前目录，因此还要重新设置当前目录为dir。  
	//执行过_findfirst后，可能系统记录下了相关信息，因此改变目录  
	//对_findnext没有影响。  
	_chdir(dir);
	if ((hFile = _findfirst("*.*", &fileinfo)) != -1)
	{
		do
		{
			//检查是不是目录  
			//如果是,再检查是不是 . 或 ..   
			//如果不是,进行迭代  
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
		return false; /** 如果不能找到第一个文件，那么没有目录 */
	}
	else
	{
		FindClose(hFile);
		return true;
	}
}

BOOL CBrowseDir::DeleteDirectory(const char *DirName)
{
	char szCurPath[MAX_PATH] = {0};					//用于定义搜索格式
	_snprintf_s(szCurPath, MAX_PATH, "%s//*.*", DirName);    //匹配格式为*.*,即该目录下的所有文件
	WIN32_FIND_DATAA FindFileData;
	ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));
	HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData);
	BOOL IsFinded = true;
	while (IsFinded)
	{
		IsFinded = FindNextFileA(hFile, &FindFileData);    //递归搜索其他的文件
		if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, "..")) //如果不是"." ".."目录
		{
			std::string strFileName = "";
			strFileName = strFileName + DirName + "//" + FindFileData.cFileName;
			std::string strTemp;
			strTemp = strFileName;
			if (IsDirectory(strFileName.c_str())) //如果是目录，则递归地调用
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
	if (bRet == 0) //删除目录
	{
		return FALSE;
	}
	return TRUE;
}

// 创建目录
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