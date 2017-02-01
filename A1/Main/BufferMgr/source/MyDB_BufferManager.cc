
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include <string>

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr whichTable, long i) {
	string targetPage = pageIdGenerator(whichTable, i);
	//page already in the buffer
	if(inbuffer(targetPage)){
		MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(page_map[targetPage]);
		this->LRU_table.erase(page_map[targetPage]);
		this->page_map[targetPage]->setLruNumber(++currentLRU);
		this->LRU_table.insert(page_map[targetPage]);	
		return handle;
	}
	//page not in the buffer
	else{
		shared_ptr <MyDB_Page> newPage = make_shared <MyDB_Page>(++currentLRU,false,false,whichTable,i,this->pageSize,this);
		string page_id = pageIdGenerator(newPage->table, newPage->pagePos);
		//buffer is not filled

		if(this->LRU_table.size() < this->pageNum){
			newPage-> bufferInfo = this->buffer +  LRU_table.size() * this->pageSize;
			newPage->loadFromFile();
		}
		//buffer is filled;
		else{
			evict(newPage);
		}
		this->page_map.insert(pair<string, shared_ptr<MyDB_Page>>(page_id,newPage));
		this->LRU_table.insert(newPage); 
		MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(newPage);
		return handle;
	}	
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	long tmpPagePos;
	if(tmp_avail.size() <= 0){
		tmpPagePos = ++tmpFileCount;
	}
	else{
		tmpPagePos = *(tmp_avail.begin());
		tmp_avail.erase(tmpPagePos);
	}
	string page_id = pageIdGenerator(tmp_table, tmpPagePos);

	shared_ptr <MyDB_Page> newPage = make_shared <MyDB_Page>(++currentLRU,false,true,this->tmp_table,tmpFileCount,this->pageSize,this);
	
	//buffer is not filled
	if(this->LRU_table.size() < this->pageNum){
		newPage-> bufferInfo = this->buffer +  LRU_table.size() * this->pageSize;
		newPage->loadFromFile();
	}
	//buffer is filled;
	else{
		evict(newPage);
	}
	this->page_map.insert(pair<string, shared_ptr<MyDB_Page>>(page_id,newPage));
	this->LRU_table.insert(newPage); 
	MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(newPage);
	return handle;	
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr whichTable, long i) {
	string targetPage = pageIdGenerator(whichTable, i);
	//page already in the buffer
	if(inbuffer(targetPage)){
		MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(page_map[targetPage]);
		this->page_map[targetPage]->setPinned();
		this->LRU_table.erase(page_map[targetPage]);
		this->page_map[targetPage]->setLruNumber(++currentLRU);
		this->LRU_table.insert(page_map[targetPage]);
		return handle;
	}
	//page not in the buffer
	else{
		shared_ptr <MyDB_Page> newPage = make_shared <MyDB_Page>(++currentLRU,true,false,whichTable,i,this->pageSize,this);
		string page_id = pageIdGenerator(newPage->table, newPage->pagePos);
		//buffer is not filled

		if(this->LRU_table.size() < this->pageNum){
			newPage-> bufferInfo = this->buffer +  LRU_table.size() * this->pageSize;
			newPage->loadFromFile();
		}
		//buffer is filled;
		else{
			evict(newPage);
		}
		this->page_map.insert(pair<string, shared_ptr<MyDB_Page>>(page_id,newPage));
		this->LRU_table.insert(newPage); 
		MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(newPage);
		return handle;
	}		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	long tmpPagePos;
	if(tmp_avail.size() <= 0){
		tmpPagePos = ++tmpFileCount;
	}
	else{
		tmpPagePos = *(tmp_avail.begin());
		tmp_avail.erase(tmpPagePos);
	}
	string page_id = pageIdGenerator(tmp_table, tmpPagePos);

	shared_ptr <MyDB_Page> newPage = make_shared <MyDB_Page>(++currentLRU,true,true,this->tmp_table,tmpFileCount,this->pageSize,this);
	
	//buffer is not filled
	if(this->LRU_table.size() < this->pageNum){
		newPage-> bufferInfo = this->buffer +  LRU_table.size() * this->pageSize;
		newPage->loadFromFile();
	}
	//buffer is filled;
	else{
		evict(newPage);
	}
	this->page_map.insert(pair<string, shared_ptr<MyDB_Page>>(page_id,newPage));
	this->LRU_table.insert(newPage); 
	MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(newPage);
	return handle;	
}
void MyDB_BufferManager :: evict(shared_ptr <MyDB_Page> newPage){
		//find the page to be evicted
		LRUtable::iterator it;  
	    for(it=LRU_table.begin();it!=LRU_table.end();it++){
	    	if((*it)->pinned == false){
	    		break;
	    	}	  
		}
		//set the status inbuffer to false
		(*it)->inBuffer = false;
		//evict from look up map
		string evict_page = pageIdGenerator((*it)->table, (*it)->pagePos);
		page_map.erase(evict_page);

		//write the data back to disk if dirty && change the data in the buffer 
		if((*it)->dirty){
			(*it)->writeToFile();
			(*it)->dirty = false;
		}
		newPage->bufferInfo = (*it)->bufferInfo;
		newPage->loadFromFile();

		//evict from LRU set
		LRU_table.erase(*it);	
}
void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
	unpinMe->pageObject->pinned = false;
}

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) {
	this->pageSize = pageSize;
	this->pageNum = numPages;
	this->tmpFile = tempFile;
	buffer = (char*) malloc(sizeof(char)*(pageSize*numPages));
	this->currentLRU = 0;
	this->tmpFileCount = 0;
	tmp_table = make_shared <MyDB_Table> ("temporary_table", this->tmpFile);
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
	LRUtable::iterator it;  
    for(it=LRU_table.begin();it!=LRU_table.end();it++){
    	if((*it)->dirty){
    		(*it)->writeToFile();
    		(*it)->dirty = false;
    	}	  
	}
	delete(this->buffer);
	remove(tmpFile.c_str());
}

bool MyDB_BufferManager ::inbuffer(string target){
	PageMap::iterator it = this->page_map.find(target);
   if(it == this->page_map.end()){
      return false;
   }
   else{
      return true;
   }
}

string MyDB_BufferManager :: pageIdGenerator(MyDB_TablePtr whichTable, long i){
		string table_name = whichTable-> getName();
		string pageId = table_name + to_string(i);
		return pageId;
}
#endif


