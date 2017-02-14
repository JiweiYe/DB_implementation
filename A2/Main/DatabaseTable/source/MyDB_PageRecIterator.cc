#ifndef PAGE_REC_ITER_C
#define PAGE_REC_ITER_C

#include "MyDB_PageRecIterator.h"
#include "MyDB_PageType.h"
#define HEADER_SIZE (sizeof (MyDB_PageType) + sizeof (size_t))
#define GET_OFFSET_UNTIL_END(ptr)  (*((size_t *) (((char *) ptr) + sizeof (MyDB_PageType))))

using namespace std;

MyDB_PageRecIterator :: MyDB_PageRecIterator(MyDB_RecordPtr _recPtr, MyDB_PageHandle _pageHdl){
	page_recPtr = _recPtr;
 	page_pageHandle = _pageHdl;
 	curLos = ((char*)_pageHdl->getBytes()) + HEADER_SIZE;
}

MyDB_PageRecIterator :: ~MyDB_PageRecIterator () {}

void MyDB_PageRecIterator :: getNext() {
	curLos = (char*)page_recPtr->fromBinary(curLos);
}

bool MyDB_PageRecIterator :: hasNext() {
	char* startLos = (char*)(page_pageHandle->getBytes());
	char* endLoc = startLos + GET_OFFSET_UNTIL_END(startLos);
	return endLoc > curLos;
} 

#endif