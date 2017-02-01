#ifndef PAGE_OBJ_C
#define PAGE_OBJ_C

#include <memory>
#include <iostream>
#include "MyDB_PageObj.h"
#include "MyDB_BufferManager.h"
#include "MyDB_PageHandle.h"

MyDB_Page :: MyDB_Page (unsigned long lru_number, bool pin, bool anonymous, MyDB_TablePtr table, long pagePos, size_t pageSize, MyDB_BufferManager* manager) {
	this->referenceCount = 0;
	this->lruNum = lru_number;
	this->pinned = pin;
	this->dirty = false;
	this->table = table;
	this->pagePos = pagePos;
	this->inBuffer = true;
	this->anonymous = anonymous;
	this->pageSize = pageSize;
    this->manager = manager;
}

MyDB_Page :: ~MyDB_Page(){
	this -> dirty = false;
}

void *MyDB_Page ::getBytes(){
    string target = this->table->getName()+ to_string(this->pagePos);
    if(!(this->inBuffer)){
        manager->evict(shared_from_this());
        manager->page_map.insert(pair<string, shared_ptr<MyDB_Page>>(target,shared_from_this()));
    }
    (manager->currentLRU)++;
    this->lruNum = manager->currentLRU;
    manager->LRU_table.erase((manager->page_map)[target]);
    manager->LRU_table.insert((manager->page_map)[target]); 
    return this->bufferInfo;
}

void MyDB_Page::writeToFile()
{
        /*int fd = open(this->table->getStorageLoc().c_str(), O_CREAT | O_RDWR | O_FSYNC, 0666 );
        if( fd == -1 || lseek(fd, this->pagePos * this->pageSize, SEEK_SET) == -1 || write(fd, this->bufferInfo, this->pageSize) == -1 )
        {
            string id = this->table->getName() + "_" + to_string(this->pagePos);
            fprintf(stderr, "Failed to write page \'%s\' to \'%s\'", id.c_str(), this->table->getStorageLoc().c_str());
            exit(-1);
        }
        if(close(fd) == -1)
        {
            fprintf(stderr, "Failed to close \'%s\'", this->table->getStorageLoc().c_str());
        }*/
        auto fd = open(this->table->getStorageLoc().c_str(), O_CREAT | O_RDWR | O_SYNC, 0666 );
        lseek(fd, this->pagePos * this->pageSize, SEEK_SET);
        write(fd, this->bufferInfo, this->pageSize);
        cout << this->bufferInfo << "write to file" << endl;
        close(fd);
}


void MyDB_Page::loadFromFile()
{
    
    auto fd = open(this->table->getStorageLoc().c_str(), O_CREAT | O_RDWR | O_SYNC, 0666 );
    lseek(fd, this->pagePos * this->pageSize, SEEK_SET);
    read(fd, this->bufferInfo, this->pageSize);
    cout << this->bufferInfo << " read from file" << endl;
   /* if( fd == -1 )
    {
        fprintf(stderr, "Failed to open file");
    }
    else if ( lseek(fd, this->pagePos * this->pageSize, SEEK_SET) == -1 )
    {
        fprintf(stderr, "Failed to use lseek");
    }
    else if ( read(fd, this->bufferInfo, this->pageSize) == -1 )
    {
        fprintf(stderr, "Failed to read from file");
    }*/
    
    close(fd);
}

void MyDB_Page::setPinned(){
	this->pinned = true;
}

void MyDB_Page::wroteBytes(){
	this->dirty = true;
}

void MyDB_Page::decreaseReference(){
	this->referenceCount--;
}

void MyDB_Page::increaseReference(){
	this->referenceCount++;
}

unsigned long MyDB_Page::getLruNumber(){
	return this->lruNum;
}

void MyDB_Page::setLruNumber(unsigned long a){
	this->lruNum = a;
}

#endif