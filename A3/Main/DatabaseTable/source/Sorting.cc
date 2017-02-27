
#ifndef SORT_C
#define SORT_C

#include <queue>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIterator.h"
#include "MyDB_TableRecIteratorAlt.h"
#include "MyDB_TableReaderWriter.h"
#include "IteratorComparator.h"
#include "Sorting.h"

using namespace std;

void mergeIntoFile (MyDB_TableReaderWriter &sortIntoMe, vector <MyDB_RecordIteratorAltPtr> &mergeUs, 
	function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

	// create the comparator and the priority queue
	IteratorComparator temp (comparator, lhs, rhs);
	priority_queue <MyDB_RecordIteratorAltPtr, vector <MyDB_RecordIteratorAltPtr>, IteratorComparator> pq (temp);

	// load up the set
	for (MyDB_RecordIteratorAltPtr m : mergeUs) {
		if (m->advance ()) {
			pq.push (m);
		}
	}

	// and write everyone out
	int counter = 0;
	while (pq.size () != 0) {

		// write the dude to the output
		auto myIter = pq.top ();
		myIter->getCurrent (lhs);
		sortIntoMe.append (lhs);
		counter++;

		// remove from the q
		pq.pop ();

		// re-insert
		if (myIter->advance ()) {
			pq.push (myIter);
		}
	}
}

void appendRecord (MyDB_PageReaderWriter &curPage, vector <MyDB_PageReaderWriter> &returnVal, 
	MyDB_RecordPtr appendMe, MyDB_BufferManagerPtr parent) {

	// try to append to the current page
	if (!curPage.append (appendMe)) {

		// if we cannot, then add a new one to the output vector
		returnVal.push_back (curPage);
		MyDB_PageReaderWriter temp (*parent);
		temp.append (appendMe);
		curPage = temp;
	}
}

vector <MyDB_PageReaderWriter> mergeIntoList (MyDB_BufferManagerPtr parent, MyDB_RecordIteratorAltPtr leftIter, 
	MyDB_RecordIteratorAltPtr rightIter, function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
	
	vector <MyDB_PageReaderWriter> returnVal;
	MyDB_PageReaderWriter curPage (*parent);
	bool lhsLoaded = false, rhsLoaded = false;

	// if one of the runs is empty, get outta here
	if (!leftIter->advance ()) {
		while (rightIter->advance ()) {
			rightIter->getCurrent (rhs);
			appendRecord (curPage, returnVal, rhs, parent);
		}
	} else if (!rightIter->advance ()) {
		while (leftIter->advance ()) {
			leftIter->getCurrent (lhs);
			appendRecord (curPage, returnVal, lhs, parent);
		}
	} else {
		while (true) {
	
			// get the two records

			// here's a bit of an optimization... if one of the records is loaded, don't re-load
			if (!lhsLoaded) {
				leftIter->getCurrent (lhs);
				lhsLoaded = true;
			}

			if (!rhsLoaded) {
				rightIter->getCurrent (rhs);		
				rhsLoaded = true;
			}
	
			// see if the lhs is less
			if (comparator ()) {
				appendRecord (curPage, returnVal, lhs, parent);
				lhsLoaded = false;

				// deal with the case where we have to append all of the right records to the output
				if (!leftIter->advance ()) {
					appendRecord (curPage, returnVal, rhs, parent);
					while (rightIter->advance ()) {
						rightIter->getCurrent (rhs);
						appendRecord (curPage, returnVal, rhs, parent);
					}
					break;
				}
			} else {
				appendRecord (curPage, returnVal, rhs, parent);
				rhsLoaded = false;

				// deal with the ase where we have to append all of the right records to the output
				if (!rightIter->advance ()) {
					appendRecord (curPage, returnVal, lhs, parent);
					while (leftIter->advance ()) {
						leftIter->getCurrent (lhs);
						appendRecord (curPage, returnVal, lhs, parent);
					}
					break;
				}
			}
		}
	}
	
	// remember the current page
	returnVal.push_back (curPage);
	
	// outta here!
	return returnVal;
}
	
void sort (int runsize, MyDB_TableReaderWriter & sortMe, MyDB_TableReaderWriter &sortIntoMe,
	function <bool ()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

	queue<MyDB_RecordIteratorAltPtr> curRun;
	vector<MyDB_RecordIteratorAltPtr> runs;
	MyDB_BufferManagerPtr parent = sortMe.getBufferMgr();

	// sort phase
	for(int i=0;i<sortMe.getNumPages();i++){

		// load a sorted page
		curRun.push(sortMe[i].sort(comparator,lhs,rhs)->getIteratorAlt());

		// if runsize pages loaded, merge them into a run
		if(curRun.size()==runsize||i==sortMe.getNumPages()-1){
			while(curRun.size()>1){
				auto first = curRun.front();
				curRun.pop();
				auto second = curRun.front();
				curRun.pop();
				auto mergeTmp = mergeIntoList(parent,first,second,comparator,lhs,rhs);
				curRun.push(getIteratorAlt(mergeTmp));
			}
			runs.push_back(curRun.front());
			curRun.pop();
		}
	}

	// merge all runs
	mergeIntoFile (sortIntoMe, runs, comparator, lhs, rhs);
}

#endif
