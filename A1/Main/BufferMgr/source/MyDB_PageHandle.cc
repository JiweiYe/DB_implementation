
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"
#include "MyDB_BufferManager.h"
void *MyDB_PageHandleBase :: getBytes () {
	return pageObject->getBytes();
}

void MyDB_PageHandleBase :: wroteBytes () {
	pageObject->wroteBytes();
}

MyDB_PageHandleBase:: MyDB_PageHandleBase(shared_ptr<MyDB_Page> object){
	this->pageObject = object;
	this->pageObject->increaseReference();
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
	pageObject->decreaseReference();
	if(pageObject->referenceCount == 0 && pageObject->pinned == true){
		pageObject->pinned = false;
	}
	if(pageObject->referenceCount == 0 && pageObject->anonymous == true){
		pageObject->manager->tmp_avail.insert(pageObject->pagePos);
		}
	}

void MyDB_PageHandleBase :: unpin(){
	pageObject->pinned = false;
}



unsigned long MyDB_PageHandleBase ::getLruNumber(){
	return pageObject->getLruNumber();
}
#endif

