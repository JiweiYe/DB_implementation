#ifndef TABLE_REC_ITER_H
#define TABLE_REC_ITER_H


#include "MyDB_TableReaderWriter.h"
#include "MyDB_RecordIterator.h"

class MyDB_TableRecIterator : public MyDB_RecordIterator{

public:
MyDB_TableReaderWriter &table_TabRW;
MyDB_RecordIteratorPtr table_PageRecIter;
MyDB_RecordPtr table_RecPtr;
size_t curPageLoc; 
void getNext() override;
bool hasNext() override;
MyDB_TableRecIterator(MyDB_TableReaderWriter&, MyDB_RecordPtr);
~MyDB_TableRecIterator();
private:

};


#endif