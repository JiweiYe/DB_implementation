
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include "MyDB_Catalog.h"
#include <unordered_map>
#include <string>
#include <vector>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;
enum Type {BOOL_TYPE, NUMERIC_TYPE, STRING_TYPE, TERMINAL };
// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:
	virtual string toString () = 0;
	virtual ~ExprTree () {}
	virtual Type getType() = 0;
	virtual bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap) { return true; };
};

class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}

	Type getType(){
		return Type::BOOL_TYPE;
	}	
};

class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}	

	~DoubleLiteral () {}

	Type getType(){
		return Type::NUMERIC_TYPE;
	}
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

	~IntLiteral () {}

	Type getType(){
		return Type::NUMERIC_TYPE;
	}
};

class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
	}

	string toString () {
		return "string[" + myVal + "]";
	}

	~StringLiteral () {}

	Type getType(){
		return Type::STRING_TYPE;
	}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
	Type type;
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
		type = Type::TERMINAL;
	}

	string toString () {
		return "[" + tableName + "_" + attName + "]";
	}	

	~Identifier () {}

	Type getType(){
		return type;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//check whether the alias exists or not
		if(alisaTableMap.find(tableName) == alisaTableMap.end()){
			cout << "Error: This alias: " << tableName << " for attribute:" << attName << " is not recorded." << endl;
			return false;
		}

		//if yes, get the original name of the table
		string tableOriginName = alisaTableMap[tableName];
		string attType;

		//check whether the attribute exists in the table above
		bool found = myCatalog->getString(tableOriginName + "." + attName + ".type", attType);
		if(!found){
			cout << "Error: The attribute " << attName << "doesn't exist in table " << tableOriginName << endl;
			return false;
		}
		else{
			if(attType.compare("int") == 0 || attType.compare("double") == 0){
				type = Type::NUMERIC_TYPE;
			}
			else if(attType.compare("string") == 0){
				type = Type::STRING_TYPE;
			}
			else{
				type = Type::BOOL_TYPE;
			}
		}
		return true;
	}
};

class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	Type getType(){
		return Type::NUMERIC_TYPE;
	}

	~MinusOp () {}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!lhs->validCheck(myCatalog,alisaTableMap) || !rhs->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//lhs shoube be of type numeric
		if(lhs->getType() != Type::NUMERIC_TYPE){
			cout << "Error: Minus Opeartion - The left hand side (" << lhs->toString() << ") is not Numeric Type" << endl; 
			return false;
		}

		//rhs shoube be of type numeric
		if(rhs->getType() != Type::NUMERIC_TYPE){
			cout << "Error: Minus Opeartion - The right hand side (" << rhs->toString() <<") is not Numeric Type" << endl; 
			return false;
		}
		return true;
	}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	Type type;
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		type = NUMERIC_TYPE;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~PlusOp () {}

	Type getType(){
		return type;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!lhs->validCheck(myCatalog,alisaTableMap) || !rhs->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		if(lhs->getType() != rhs->getType()){
			cout << "Error: Plus Operation - The type of left hand side(" << lhs->toString() << ") and the type of right hand side(" << rhs->toString() + ") is not the same." << endl;
			return false;
		}

		if(lhs->getType() == Type::BOOL_TYPE){
			cout << "Error: Plus Operation - Boolean Type is not allowed in plus Operation." << endl;
			return false;
		}

		else if(lhs->getType() == Type::STRING_TYPE){
			type = Type::STRING_TYPE;
			return true;
		}

		else if(lhs->getType() == Type::NUMERIC_TYPE){
			type = Type::NUMERIC_TYPE;
			return true;
		}
	}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~TimesOp () {}

	Type getType(){
		return Type::NUMERIC_TYPE;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!lhs->validCheck(myCatalog,alisaTableMap) || !rhs->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//lhs shoube be of type numeric
		if(lhs->getType() != Type::NUMERIC_TYPE){
			cout << "Error: Times Opeartion - The left hand side (" << lhs->toString() <<") is not Numeric Type" << endl; 
			return false;
		}

		//rhs shoube be of type numeric
		if(rhs->getType() != Type::NUMERIC_TYPE){
			cout << "Error: Times Opeartion - The right hand side (" << rhs->toString() <<") is not Numeric Type" << endl; 
			return false;
		}
		return true;
	}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~DivideOp () {}

	Type getType(){
		return Type::NUMERIC_TYPE;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!lhs->validCheck(myCatalog,alisaTableMap) || !rhs->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//lhs shoube be of type numeric
		if(lhs->getType() != Type::NUMERIC_TYPE){
			cout << "Error: Divide Opeartion - The left hand side (" << lhs->toString() <<") is not Numeric Type" << endl; 
			return false;
		}

		//rhs shoube be of type numeric
		if(rhs->getType() != Type::NUMERIC_TYPE){
			cout << "Error: Divide Opeartion - The right hand side (" << rhs->toString() <<") is not Numeric Type" << endl; 
			return false;
		}
		return true;
	}
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~GtOp () {}

	Type getType(){
		return Type::BOOL_TYPE;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!lhs->validCheck(myCatalog,alisaTableMap) || !rhs->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//lhs and rhs should be of same type
		if(lhs->getType() != rhs->getType()){
			cout << "Error: Greater Than Operation - The type of lhs hand side(" << lhs->toString() <<") is not the same as that of right hand side(" << rhs->toString() << ")." << endl;
			return false;
		}

		return true;
	}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~LtOp () {}

	Type getType(){
		return Type::BOOL_TYPE;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!lhs->validCheck(myCatalog,alisaTableMap) || !rhs->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//lhs and rhs should be of same type
		if(lhs->getType() != rhs->getType()){
			cout << "Error: Less Than Operation - The type of lhs hand side(" << lhs->toString() <<") is not the same as that of right hand side(" << rhs->toString() << ")." << endl;
			return false;
		}

		return true;
	}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~NeqOp () {}

	Type getType(){
		return Type::BOOL_TYPE;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!lhs->validCheck(myCatalog,alisaTableMap) || !rhs->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//lhs and rhs should be of same type
		if(lhs->getType() != rhs->getType()){
			cout << "Error: Not Equal Operation - The type of lhs hand side(" << lhs->toString() <<") is not the same as that of right hand side(" << rhs->toString() << ")." << endl;
			return false;
		}

		return true;
	}
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~OrOp () {}

	Type getType(){
		return Type::BOOL_TYPE;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!lhs->validCheck(myCatalog,alisaTableMap) || !rhs->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//lhs should be of type boolean
		if(lhs->getType() != Type::BOOL_TYPE){
			cout << "Error: Or Operation - The type of lhs hand side(" << lhs->toString() <<") is not the type of Boolean" << endl;
			return false;
		}

		//lhs should be of type boolean
		if(rhs->getType() != Type::BOOL_TYPE){
			cout << "Error: Or Operation - The type of rhs hand side(" << rhs->toString() <<") is not the type of Boolean" << endl;
			return false;
		}

		return true;
	}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	~EqOp () {}

	Type getType(){
		return Type::BOOL_TYPE;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!lhs->validCheck(myCatalog,alisaTableMap) || !rhs->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//lhs and rhs should be of same type
		if(lhs->getType() != rhs->getType()){
			cout << "Error: Equal Operation - The type of lhs hand side(" << lhs->toString() +") is not the same as that of right hand side(" << rhs->toString() << ")." << endl;
			if(lhs->getType() == Type::BOOL_TYPE){
				cout << "lhs:bool" <<endl;
			}
			else if(lhs->getType() == Type::NUMERIC_TYPE){
				cout << "lhs:num" << endl;
			}
			else if(lhs->getType() == Type::STRING_TYPE){
				cout << "lhs:string" << endl;
			}
			else if(lhs->getType() == Type::TERMINAL){
				cout << "lhs:terminal" << endl;
			}
			if(rhs->getType() == Type::BOOL_TYPE){
				cout << "rhs:bool" <<endl;
			}
			else if(rhs->getType() == Type::NUMERIC_TYPE){
				cout << "rhs:num" << endl;
			}
			else if(rhs->getType() == Type::STRING_TYPE){
				cout << "rhs:string" << endl;
			}
			else if(rhs->getType() == Type::TERMINAL){
				cout << "rhs:terminal" << endl;
			}
			return false;
		}

		return true;
	}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}	

	~NotOp () {}

	Type getType(){
		return Type::BOOL_TYPE;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//child should be valid
		if(!child->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//child should be of type boolean
		if(child->getType() != Type::BOOL_TYPE){
			cout << "Error: Not Operation - The type of child(" << child->toString() <<") is not the type of Boolean" << endl;
			return false;
		}

		return true;
	}
};

class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}	

	~SumOp () {}

	Type getType(){
		return Type::NUMERIC_TYPE;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!child->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//child should be of type numeric
		if(child->getType() != Type::NUMERIC_TYPE){
			cout << "Error: Sum Operation - The type of child(" << child->toString() <<") is not the type of Numeric" << endl;
			return false;
		}

		return true;
	}

};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}	

	~AvgOp () {}

	Type getType(){
		return Type::NUMERIC_TYPE;
	}

	bool validCheck(MyDB_CatalogPtr myCatalog, unordered_map<string, string>& alisaTableMap){
		//both lhs and rhs should be valid
		if(!child->validCheck(myCatalog,alisaTableMap)){
			return false;
		}

		//child should be of type numeric
		if(child->getType() != Type::NUMERIC_TYPE){
			cout << "Error: Sum Operation - The type of child(" << child->toString() <<") is not the type of Numeric" << endl;
			return false;
		}

		return true;
	}
};

#endif
