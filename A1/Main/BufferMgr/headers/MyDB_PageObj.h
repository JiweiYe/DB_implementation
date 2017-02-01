#ifndef PAGE_OBJ_H
#define PAGE_OBJ_H

#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "MyDB_Table.h"
using namespace std;
class MyDB_BufferManager;
class MyDB_Page : public enable_shared_from_this<MyDB_Page>{

public:
	char* bufferInfo;
	int referenceCount;
	unsigned long lruNum;
	bool anonymous;
	bool pinned;
	bool dirty;
	MyDB_TablePtr table;
	long pagePos;
	bool inBuffer;
	size_t pageSize;
	MyDB_BufferManager* manager;
	MyDB_Page(unsigned long lru_number, bool pin, bool anonymous, MyDB_TablePtr table, long pagePos, size_t pageSize, MyDB_BufferManager* manager);
	~MyDB_Page();
	void setPinned();
	void setDirty();
	void* getBytes();
	void wroteBytes();
	void increaseReference();
	void decreaseReference();
	unsigned long getLruNumber();
	void setLruNumber(unsigned long a);
	void loadFromFile();
	void writeToFile();


private:
};

#endif