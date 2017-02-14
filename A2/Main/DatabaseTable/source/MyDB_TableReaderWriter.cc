
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include <fstream>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_TableRecIterator.h"
using namespace std;

MyDB_TableReaderWriter :: MyDB_TableReaderWriter (MyDB_TablePtr tablePtr, MyDB_BufferManagerPtr bmPtr) {
	bufferPtr = bmPtr;
	tabPtr = tablePtr;
}

MyDB_PageReaderWriter &MyDB_TableReaderWriter :: operator [] (size_t index) {
	curPage = make_shared<MyDB_PageReaderWriter>(*this, index);
	return *curPage;
}

MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord () {
	MyDB_RecordPtr anEmptyRec = make_shared<MyDB_Record>(tabPtr->getSchema());
	return anEmptyRec;
}

MyDB_PageReaderWriter &MyDB_TableReaderWriter :: last () {
	return (*this)[tabPtr->lastPage()];
}


void MyDB_TableReaderWriter :: append (MyDB_RecordPtr ptr) {
	while(!endPage->append(ptr)){
		tabPtr->setLastPage(tabPtr->lastPage() + 1);
		endPage = make_shared<MyDB_PageReaderWriter>(*this, tabPtr->lastPage());
		endPage->clear();
	}
}

void MyDB_TableReaderWriter :: loadFromTextFile (string fileName) {
	std::ifstream textFile(fileName);
	string record;

	tabPtr->setLastPage(0);
	endPage = make_shared<MyDB_PageReaderWriter>(*this, tabPtr->lastPage());
	endPage ->clear();
	MyDB_RecordPtr emptyRec = getEmptyRecord();
	while(std::getline(textFile, record)){
		emptyRec->fromString(record);
		append(emptyRec);
	}
	textFile.close();

}

MyDB_RecordIteratorPtr MyDB_TableReaderWriter :: getIterator (MyDB_RecordPtr recPtr) {
	return make_shared<MyDB_TableRecIterator> (*this, recPtr);
}

void MyDB_TableReaderWriter :: writeIntoTextFile (string fileName) {
	std::ofstream toText(fileName);
	MyDB_RecordPtr aRec = getEmptyRecord();
	MyDB_RecordIteratorPtr record_itr = getIterator(aRec); 
	while(record_itr->hasNext()){
		record_itr->getNext();
		toText << aRec << '\n';
	}
	toText.close();
}

#endif

