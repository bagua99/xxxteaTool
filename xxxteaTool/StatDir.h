#include "BrowseDir.h"

//从CBrowseDir派生出的子类，用来统计目录中的文件及子目录个数  
class CStatDir :public CBrowseDir
{
public:
	//缺省构造器  
	CStatDir();

	//返回文件个数  
	int GetFileCount();

	//返回子目录个数  
	int GetSubdirCount();

protected:
	//覆写虚函数ProcessFile，每调用一次，文件个数加1  
	bool ProcessFile(const char *filename);

	//覆写虚函数ProcessDir，每调用一次，子目录个数加1  
	void ProcessDir(const char *currentdir, const char *parentdir);

private:
	int m_nFileCount;   //保存文件个数  
	int m_nSubdirCount; //保存子目录个数 
};