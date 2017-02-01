
#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include <map>
#include <set>
#include <stdlib.h>
#include "MyDB_PageHandle.h"
#include "MyDB_Table.h"


using namespace std;

struct PageComparator
{
  bool operator()(const shared_ptr<MyDB_Page> s1, const shared_ptr<MyDB_Page> s2) const
  {
    return s1->getLruNumber() < s2->getLruNumber();
  }
};
typedef map<string, shared_ptr<MyDB_Page>> PageMap;
//typedef map<unsigned long, string> LRUtable;
typedef set<shared_ptr<MyDB_Page>,PageComparator> LRUtable;


class MyDB_BufferManager {

public:
	set<long> tmp_avail;
	MyDB_TablePtr tmp_table;
	long tmpFileCount;
	char* buffer;
	size_t pageSize;
	size_t pageNum;
	string tmpFile;
	PageMap page_map;
	LRUtable LRU_table;
	unsigned long currentLRU;
	string pageIdGenerator(MyDB_TablePtr whichTable, long i);
	bool inbuffer(string);
	void evict(shared_ptr <MyDB_Page> newPage);
	// THESE METHODS MUST APPEAR AND THE PROTOTYPES CANNOT CHANGE!

	// gets the i^th page in the table whichTable... note that if the page
	// is currently being used (that is, the page is current buffered) a handle 
	// to that already-buffered page should be returned
	MyDB_PageHandle getPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page that will no longer exist (1) after the buffer manager
	// has been destroyed, or (2) there are no more references to it anywhere in the
	// program.  Typically such a temporary page will be used as buffer memory.
	// since it is just a temp page, it is not associated with any particular 
	// table
	MyDB_PageHandle getPage ();

	// gets the i^th page in the table whichTable... the only difference 
	// between this method and getPage (whicTable, i) is that the page will be 
	// pinned in RAM; it cannot be written out to the file
	MyDB_PageHandle getPinnedPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page, like getPage (), except that this one is pinned
	MyDB_PageHandle getPinnedPage ();

	// un-pins the specified page
	void unpin (MyDB_PageHandle unpinMe);

	// creates an LRU buffer manager... params are as follows:
	// 1) the size of each page is pageSize 
	// 2) the number of pages managed by the buffer manager is numPages;
	// 3) temporary pages are written to the file tempFile
	MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile);
	
	// when the buffer manager is destroyed, all of the dirty pages need to be
	// written back to disk, any necessary data needs to be written to the catalog,
	// and any temporary files need to be deleted
	~MyDB_BufferManager ();

	// FEEL FREE TO ADD ADDITIONAL PUBLIC METHODS 

private:

	// YOUR STUFF HERE

};

#endif


