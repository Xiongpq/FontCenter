#pragma once
using namespace std;
#include <vector>

class FontBLL
{
public:
	FontBLL();
	~FontBLL();
	static unsigned __stdcall run(void * pThis);
	void sync_font();
};
