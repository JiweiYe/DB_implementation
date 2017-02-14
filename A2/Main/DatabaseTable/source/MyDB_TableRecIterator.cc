
#ifndef TABLE_REC_ITER_C
#define TABLE_REC_ITER_C

#include "MyDB_TableRecIterator.h"
#include "MyDB_PageReaderWriter.h"
using namespace std;

MyDB_TableRecIterator :: MyDB_TableRecIterator(MyDB_TableReaderWriter& tbRecIt, MyDB_RecordPtr RecPtr): table_TabRW(tbRecIt){
	table_RecPtr = RecPtr;
	curPageLoc = 0;
	table_PageRecIter = tbRecIt[curPageLoc].getIterator(table_RecPtr);
}

MyDB_TableRecIterator:: ~MyDB_TableRecIterator(){};

void MyDB_TableRecIterator:: getNext(){
	table_PageRecIter->getNext();
}

bool MyDB_TableRecIterator:: hasNext(){
	while(!table_PageRecIter->hasNext()){
		if(curPageLoc == table_TabRW.tabPtr->lastPage()){
			return false;
		}
		curPageLoc++;
		table_PageRecIter = table_TabRW[curPageLoc].getIterator(table_RecPtr);
	}
	return table_PageRecIter->hasNext();
}
#endif