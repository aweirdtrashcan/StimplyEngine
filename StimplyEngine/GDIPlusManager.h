#pragma once

#include "includes.h"

class GDIPlusManager
{
public:
	GDIPlusManager();
	~GDIPlusManager();
private:
	inline static ULONG_PTR token;
	inline static int refCount;
};

