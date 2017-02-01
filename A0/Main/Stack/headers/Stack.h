 
#ifndef STACK_H
#define STACK_H

// this is the node class used to build up the LIFO stack
template <class Data>
class Node {

private:

	Data holdMe;
	Node *next;
	
public:
	/*****************************************/
	/** WHATEVER CODE YOU NEED TO ADD HERE!! */
	/*****************************************/
	Node(Data data) : holdMe(data){
	 	next = NULL;
	}

	Node* getNext(){
		return next;
	}

	Data getData(){
		return holdMe;
	}

	void setNext(Node* node){
		next = node;
	}
};

// a simple LIFO stack
template <class Data> 
class Stack {

	Node <Data> *head;
public:

	// destroys the stack
	~Stack () { 
		Node <Data> *tmp;
		while(head != NULL){
			tmp = head->getNext();
			delete(head);
			head = tmp;
		}
		// tmp = NULL;
		// head = NULL;
	/* your code here */ }

	// creates an empty stack
	Stack () { /* your code here */
		head = NULL;
	}

	// adds pushMe to the top of the stack
	void push (Data node) { 
		Node <Data> *tmpNode = new Node<Data> (node);
		tmpNode->setNext(head);
		head = tmpNode;
	/* your code here */ }

	// return true if there are not any items in the stack
	bool isEmpty () { 
	/* replace with your code */ 
	if(head != NULL){
		return false; 
	}

	else{
		return true;
	}

	}

	// pops the item on the top of the stack off, returning it...
	// if the stack is empty, the behavior is undefined
	Data pop () { /* replace with your code */ 
		Data returnData = head->getData();
		Node <Data> *returnNode = head;
		head = head->getNext();
		delete(returnNode);
		return returnData;
	}
};

#endif
