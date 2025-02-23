#ifndef JSON_H
#define JSON_H
#include <stdio.h>

//arraylist
typedef struct {
	//the size of each element in the array list
	int size;
	
	//the length of the list
	int length;
	
	//the data in the list
	void* data;
} arraylist;

//an object with key value pairs
typedef struct {
	//an arrayList of keys
	arraylist* keys;
	
	//an arraylist of values 
	arraylist* values;
} json_nodeObject;

//has the data of any given json_nodeHead
typedef union {
	// NOTE: if the data is null, the type in the nodeHead should be json_NULL

	//if the data is a number
	int number;
	
	//if the data is a number with a decimal
	double fraction;
	
	//if the data is a bool value
	char boolean;

	//if the data is a string
	// NOTE: during parsing, this is used to exponents before converting them to numbers
	char* string;
	
	//if the data is an array
	arraylist* array;

	//if the data is an object
	json_nodeObject object;

} json_nodeBody;

//has the type of data in a json node as well as a pointer to the data itself
typedef struct {
	int type;
	json_nodeBody data;
} json_nodeHead;

typedef json_nodeHead json;

//parses a json file and creates a tree of nodes
json* json_parseFromPath(char* pathToFile);

//parses a json file and creates a tree of nodes
json* json_parse(FILE* fptr);

//frees the data of a json pointer, and sets the ptr to NULL
void json_remove(json** jsonPtr);

//gets the int value of a json pointer
int json_getInt(json* val);

//gets the double value of a json pointer
double json_getDouble(json* val);

//gets the string value of a json pointer
char* json_getString(json* val);

//gets the json pointer at a specified index of a json pointer of type array
json* json_getIndex(json* val, int index);

//gets the value of the specified member of a json object
json* json_getMember(json* val, char* memberName);

//gets the boolean value of a json ptr
char json_getBool(json* val);

//gets the length of a json array
int json_arrayLen(json* val);

//checks if type is int
char json_isInt(json* val);

//checks if type is double
char json_isDouble(json* val);

//checks if type is string
char json_isString(json* val);

//checks if type is array
char json_isArray(json* val);

//checks if type is object
char json_isObject(json* val);

//checks if type is boolean 
char json_isBool(json* val);

//checks if type is null
char json_isNull(json* val);

//checks if type is invalid
char json_isInvalid(json* val);

#endif
