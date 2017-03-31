
#ifndef BPLUS_C
#define BPLUS_C

#include <queue>
#include <algorithm>
#include "MyDB_INRecord.h"
#include "MyDB_BPlusTreeReaderWriter.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_PageListIteratorSelfSortingAlt.h"
#include "RecordComparator.h"

MyDB_BPlusTreeReaderWriter :: MyDB_BPlusTreeReaderWriter (string orderOnAttName, MyDB_TablePtr forMe, 
	MyDB_BufferManagerPtr myBuffer) : MyDB_TableReaderWriter (forMe, myBuffer) {

	// find the ordering attribute
	auto res = forMe->getSchema ()->getAttByName (orderOnAttName);

	// remember information about the ordering attribute
	orderingAttType = res.second;
	whichAttIsOrdering = res.first;

	// and the root location
	rootLocation = getTable ()->getRootLocation ();
}

MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter :: getSortedRangeIteratorAlt (MyDB_AttValPtr low, MyDB_AttValPtr high) {
	vector<MyDB_PageReaderWriter> list; 
	discoverPages(rootLocation, list, low, high);

	MyDB_RecordPtr lhsIn = getEmptyRecord();
	MyDB_RecordPtr rhsIn = getEmptyRecord();
    function <bool ()> comparatorIn = buildComparator(lhsIn, rhsIn);

    MyDB_RecordPtr myRecIn = getEmptyRecord();
	MyDB_INRecordPtr lowRecord = getINRecord();
	lowRecord-> setKey(low);
	MyDB_INRecordPtr highRecord = getINRecord();
	highRecord -> setKey(high);

	function <bool ()> lowComparatorIn = buildComparator(myRecIn,lowRecord);
	function <bool ()> highComparatorIn = buildComparator(highRecord,myRecIn);

	return make_shared<MyDB_PageListIteratorSelfSortingAlt>(list, lhsIn, rhsIn, comparatorIn, myRecIn, 
		lowComparatorIn, highComparatorIn, true);
}

MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter :: getRangeIteratorAlt (MyDB_AttValPtr low, MyDB_AttValPtr high) {
	vector<MyDB_PageReaderWriter> list; 
	discoverPages(rootLocation, list, low, high);

	MyDB_RecordPtr lhsIn = getEmptyRecord();
	MyDB_RecordPtr rhsIn = getEmptyRecord();
    function <bool ()> comparatorIn = buildComparator(lhsIn, rhsIn);

    MyDB_RecordPtr myRecIn = getEmptyRecord();
	MyDB_INRecordPtr lowRecord = getINRecord();
	lowRecord-> setKey(low);
	MyDB_INRecordPtr highRecord = getINRecord();
	highRecord -> setKey(high);

	function <bool ()> lowComparatorIn = buildComparator(myRecIn,lowRecord);
	function <bool ()> highComparatorIn = buildComparator(highRecord,myRecIn);

	return make_shared<MyDB_PageListIteratorSelfSortingAlt>(list, lhsIn, rhsIn, comparatorIn, myRecIn, 
		lowComparatorIn, highComparatorIn, false);
}



bool MyDB_BPlusTreeReaderWriter :: discoverPages (int whichPage, vector <MyDB_PageReaderWriter> &list, MyDB_AttValPtr low, MyDB_AttValPtr high) {
	
	vector<MyDB_PageReaderWriter> traverseList;
	MyDB_PageReaderWriter aPage(*this,whichPage);

	if(aPage.getType() == MyDB_PageType::DirectoryPage){
		traverseList.push_back(aPage);
		auto lowRecord = getINRecord();
		auto highRecord = getINRecord();
		auto traverseRecord = getINRecord();
		lowRecord->setKey(low);
		highRecord->setKey(high);

		function <bool ()> isLower = buildComparator(traverseRecord, lowRecord);
		function <bool ()> isHigher = buildComparator(highRecord, traverseRecord);

		for(int i = 0; i < traverseList.size(); i++){
			if(traverseList[i].getType() == MyDB_PageType::RegularPage){
				list.push_back(traverseList[i]);
				continue;
			}
			MyDB_RecordIteratorAltPtr recordItr = traverseList[i].getIteratorAlt();
			while(recordItr->advance()){
				recordItr->getCurrent(traverseRecord);
				if(isLower()){
					continue;
				}
				else if(!isLower() && !isHigher()){
					MyDB_PageReaderWriter nodePage(*this, traverseRecord->getPtr());
					traverseList.push_back(nodePage);
				}
				else if(isHigher()){
					MyDB_PageReaderWriter nodePage(*this, traverseRecord->getPtr());
					traverseList.push_back(nodePage);
					break;
				}
			}
		}
		return false;
	}
	else{
		list.push_back(aPage);
		return true;
	}
}

void MyDB_BPlusTreeReaderWriter :: append (MyDB_RecordPtr appendMe) {
	if(getNumPages() < 2){
		
		MyDB_PageReaderWriter rootPage = (*this)[0];
		rootPage.clear ();
		rootPage.setType (MyDB_PageType :: DirectoryPage);
		getTable ()->setRootLocation (0);

        getTable ()->setLastPage (1);
		MyDB_PageReaderWriter leafPage = (*this)[1];
		leafPage.clear ();
		leafPage.setType (MyDB_PageType :: RegularPage);

		auto InterNodeRec = getINRecord();
		InterNodeRec->setPtr (1);
		rootPage.append (InterNodeRec);
		leafPage.append (appendMe);
		rootLocation = 0;
	}else {
		
		auto recptr = append(rootLocation, appendMe);
		if(recptr){

			int newLocation = getTable ()->lastPage () + 1;
			getTable ()->setLastPage (newLocation);
			MyDB_PageReaderWriter newRootPage = (*this)[newLocation];

			newRootPage.clear();
			newRootPage.setType(MyDB_PageType::DirectoryPage);

			auto newInterNodeRec = getINRecord();
			newInterNodeRec->setPtr(rootLocation);
			newRootPage.append(recptr);
			newRootPage.append(newInterNodeRec);

			getTable ()->setRootLocation (newLocation);
			rootLocation = newLocation;

		}
	}
}

MyDB_RecordPtr MyDB_BPlusTreeReaderWriter :: split (MyDB_PageReaderWriter splitMe, MyDB_RecordPtr addMe) {

	int newLocation = getTable ()->lastPage () + 1;
	getTable()->setLastPage (newLocation);
	auto newpage = make_shared<MyDB_PageReaderWriter>(*this, newLocation);
	newpage->clear();
	newpage->setType(splitMe.getType());
	
	auto returnPtr = getINRecord ();
	returnPtr->setPtr (newLocation);

	
	MyDB_RecordPtr lhs, rhs;
	if (splitMe.getType() == MyDB_PageType :: RegularPage){
		lhs = getEmptyRecord();
		rhs = getEmptyRecord();
	}
	else if (splitMe.getType() == MyDB_PageType :: DirectoryPage){
		lhs = getINRecord();
		rhs = getINRecord();
	}
	auto comparator = buildComparator(lhs,rhs);

	void *tempPage = malloc (splitMe.getPageSize ());
	memcpy (tempPage, splitMe.getBytes (), splitMe.getPageSize ());

    vector <void *> recVec;
    size_t pageBytes = *((size_t *) (((char *) tempPage) + sizeof (size_t)));

    int bytesConsumed = sizeof(size_t)*2;
    while(bytesConsumed != pageBytes){
    	void* rec = bytesConsumed + (char*)tempPage;
		recVec.push_back(rec);
		void* nextRec = lhs->fromBinary(rec);
		bytesConsumed += ((char*) nextRec) - ((char*) rec);
    }

    size_t recordSize = addMe->getBinarySize();
    void* addRec = malloc(recordSize);
    addMe->toBinary (addRec);
	recVec.push_back (addRec);

	RecordComparator recComparator (comparator, lhs, rhs);
	stable_sort (recVec.begin (), recVec.end (), recComparator);

	splitMe.clear();
	splitMe.setType (newpage->getType());

	int counter = 0;
	for (auto i: recVec){
	
		lhs->fromBinary(i);
		if (counter < recVec.size()/2){
			newpage->append(lhs);
		}
		else if (counter == recVec.size()/2){
			newpage->append(lhs);
			returnPtr->setKey(getKey(lhs));
		}
		else{
			splitMe.append(lhs);
		}
		
		counter++;
	}
	
	
	free(addRec);
	free(tempPage);
	
	return returnPtr;
}

MyDB_RecordPtr MyDB_BPlusTreeReaderWriter :: append (int page, MyDB_RecordPtr appendMe) {

	auto pageAppended = make_shared<MyDB_PageReaderWriter>(*this, page);

	if (pageAppended->getType() == MyDB_PageType::RegularPage){
		if(pageAppended->append(appendMe)){
			return nullptr;
		}

		auto new_internal = split(*pageAppended, appendMe);
		return new_internal;
	
	} else {
		
		auto currentIter = pageAppended->getIteratorAlt();
		auto record = getINRecord();
		auto comparator = buildComparator(appendMe, record);

		while(currentIter->advance()){

			currentIter->getCurrent(record);
			
			if(comparator()){
				
				auto rPointer = append(record->getPtr(),appendMe);
				
				if(rPointer == nullptr){
					return nullptr;
				}

				if(pageAppended -> append(rPointer)){
					auto lhs = getINRecord();
					pageAppended->sortInPlace(buildComparator(rPointer,lhs),rPointer,lhs);
					return nullptr;
				}else {

					return split(*pageAppended, rPointer);
				}
			}

		}
	}

	return nullptr;
}

MyDB_INRecordPtr MyDB_BPlusTreeReaderWriter :: getINRecord () {
	return make_shared <MyDB_INRecord> (orderingAttType->createAttMax ());
}

void MyDB_BPlusTreeReaderWriter :: printTree () {
	
	queue<MyDB_PageReaderWriter> q;
	MyDB_PageReaderWriter rootNode = (*this)[rootLocation];
	q.push(rootNode);

	while (!q.empty()){
		int length = q.size();
		for (int i= 0; i < length; i++){
			MyDB_PageReaderWriter temp = q.front();
			q.pop();
			MyDB_RecordIteratorAltPtr iterator = temp.getIteratorAlt();		
			if (temp.getType() == MyDB_PageType::RegularPage){
				MyDB_RecordPtr rec = getEmptyRecord();
				while(iterator->advance()){
					iterator->getCurrent(rec);
					cout<<rec<<"*";
				}
			}
			else if (temp.getType() == MyDB_PageType::DirectoryPage){
				MyDB_INRecordPtr rec = getINRecord();
				while(iterator->advance()){
					iterator->getCurrent(rec);
					q.push((*this)[(rec)->getPtr()]);
					cout<<rec<<"*";
				}
			}	
			cout<<"\t";
		}
		cout<<"\n";
	}
}

MyDB_AttValPtr MyDB_BPlusTreeReaderWriter :: getKey (MyDB_RecordPtr fromMe) {

	// in this case, got an IN record
	if (fromMe->getSchema () == nullptr) 
		return fromMe->getAtt (0)->getCopy ();

	// in this case, got a data record
	else 
		return fromMe->getAtt (whichAttIsOrdering)->getCopy ();
}

function <bool ()>  MyDB_BPlusTreeReaderWriter :: buildComparator (MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

	MyDB_AttValPtr lhAtt, rhAtt;

	// in this case, the LHS is an IN record
	if (lhs->getSchema () == nullptr) {
		lhAtt = lhs->getAtt (0);	

	// here, it is a regular data record
	} else {
		lhAtt = lhs->getAtt (whichAttIsOrdering);
	}

	// in this case, the LHS is an IN record
	if (rhs->getSchema () == nullptr) {
		rhAtt = rhs->getAtt (0);	

	// here, it is a regular data record
	} else {
		rhAtt = rhs->getAtt (whichAttIsOrdering);
	}
	
	// now, build the comparison lambda and return
	if (orderingAttType->promotableToInt ()) {
		return [lhAtt, rhAtt] {return lhAtt->toInt () < rhAtt->toInt ();};
	} else if (orderingAttType->promotableToDouble ()) {
		return [lhAtt, rhAtt] {return lhAtt->toDouble () < rhAtt->toDouble ();};
	} else if (orderingAttType->promotableToString ()) {
		return [lhAtt, rhAtt] {return lhAtt->toString () < rhAtt->toString ();};
	} else {
		cout << "This is bad... cannot do anything with the >.\n";
		exit (1);
	}
}


#endif
