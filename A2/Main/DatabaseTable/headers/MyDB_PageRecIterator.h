#ifndef PAGE_REC_ITER_H
#define PAGE_REC_ITER_H

#include <memory>
#include "MyDB_RecordIterator.h"
#include "MyDB_Record.h"
#include "MyDB_PageHandle.h"

using namespace std;
class MyDB_PageRecIterator : public MyDB_RecordIterator {

public:
MyDB_RecordPtr page_recPtr;
MyDB_PageHandle page_pageHandle;
char *curLos;
void getNext() override;
bool hasNext() override;
MyDB_PageRecIterator(MyDB_RecordPtr, MyDB_PageHandle);
~MyDB_PageRecIterator();

private:


};

#endif