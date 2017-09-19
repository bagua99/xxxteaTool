#include "BrowseDir.h"

//��CBrowseDir�����������࣬����ͳ��Ŀ¼�е��ļ�����Ŀ¼����  
class CStatDir :public CBrowseDir
{
public:
	//ȱʡ������  
	CStatDir();

	//�����ļ�����  
	int GetFileCount();

	//������Ŀ¼����  
	int GetSubdirCount();

protected:
	//��д�麯��ProcessFile��ÿ����һ�Σ��ļ�������1  
	bool ProcessFile(const char *filename);

	//��д�麯��ProcessDir��ÿ����һ�Σ���Ŀ¼������1  
	void ProcessDir(const char *currentdir, const char *parentdir);

private:
	int m_nFileCount;   //�����ļ�����  
	int m_nSubdirCount; //������Ŀ¼���� 
};