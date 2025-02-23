# jsonParser
A parser for json files written in C

# function descriptions:

json* json_parseFromPath(char* pathToFile);
this takes a path to a json file, parses it, and returns the result as a json pointer

json* json_parse(FILE* fptr);
this takes a file pointer to a json file, parses it, and returns the result as a json pointer

void json_remove(json** jsonPtr);
frees the data of a json pointer, and sets the ptr to NULL

int json_getInt(json* val);
gets the int value of a json pointer

double json_getDouble(json* val);
gets the double value of a json pointer

char* json_getString(json* val);
gets the string value of a json pointer

json* json_getIndex(json* val, int index);
gets the json pointer at a specified index of a json pointer of type array

json* json_getMember(json* val, char* memberName);
gets the value of the specified member of a json object

char json_getBool(json* val);
gets the boolean value of a json pointer

int json_arrayLen(json* val);
gets the length of a json array

char json_isInt(json* val);
checks if the type of a json pointer is int

char json_isDouble(json* val);
checks if the type of a json pointer is double

char json_isString(json* val);
checks if the type of a json pointer is string

char json_isArray(json* val);
checks if the type of a json pointer is array

char json_isObject(json* val);
checks if the type of a json pointer is object

char json_isBool(json* val);
checks if the type of a json pointer is boolean 

char json_isNull(json* val);
checks if the type of a json pointer is null
