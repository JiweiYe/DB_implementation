
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"
#define GET_TYPE(ptr) (*((MyDB_PageType *) ptr))
#define GET_OFFSET_UNTIL_END(ptr)  (*((size_t *) (((char *) ptr) + sizeof (MyDB_PageType))))

MyDB_PageReaderWriter :: MyDB_PageReaderWriter(MyDB_TableReaderWriter &trw, size_t index){
	pageHandle = trw.bufferPtr->getPage(trw.tabPtr,index);
	pageSize = trw.bufferPtr->getPageSize();
	setType(MyDB_PageType :: RegularPage);
}
void MyDB_PageReaderWriter :: clear () {
	*((size_t*)((char*)pageHandle->getBytes()+sizeof(MyDB_PageType))) = sizeof(MyDB_PageType) + sizeof(size_t);
	setType(MyDB_PageType :: RegularPage);
	pageHandle->wroteBytes();
}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
	return GET_TYPE((char*)pageHandle->getBytes());
}

MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (MyDB_RecordPtr rec_Ptr) {
	return make_shared <MyDB_PageRecIterator> (rec_Ptr, pageHandle);
}

void MyDB_PageReaderWriter :: setType (MyDB_PageType type) {
	GET_TYPE((char*)pageHandle->getBytes()) = type;
}

bool MyDB_PageReaderWriter :: append (MyDB_RecordPtr ptr) {
	size_t recordSize = ptr->getBinarySize();
	size_t usedSize = GET_OFFSET_UNTIL_END((char*)pageHandle->getBytes());
	if(recordSize + usedSize > pageSize){
		return false;
	}
	else{
	char* nextPos = (char*)pageHandle->getBytes() + usedSize;
	ptr->toBinary(nextPos);
	pageHandle->wroteBytes();
	GET_OFFSET_UNTIL_END((char*)pageHandle->getBytes()) += recordSize;
	return true;
	}
}

#endif
