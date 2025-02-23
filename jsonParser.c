#include "jsonParser.h"

//a copy of the bytes in a file
typedef struct {
	//the length of the file in bytes
	long long len;

	//a copy of the bytes in the file
	char* bytes;
} filecopy;

enum json_typeFlags {
	
	//is a number
	json_NUMBER = 0x1,
	
	//is a number with a decimal
	json_DECIMAL = 0x3,
	
	//is a number ending in an exponent
	// NOTE: this should only be used for parsing purposes
	json_EXPONENT = 0x7,
	
	//is a negative number
	json_NEGATIVE = 0x9,
	
	//is a boolean value
	// NOTE: this flag should not be assigned as a type on its own
	json_BOOLEAN = 0x10,

	//is the boolean value of true
	json_TRUE = 0x30,
	
	//is the boolean value of false
	json_FALSE = 0x50,
	
	//is the value null
	// WARNING: json_NULL is not the same as NULL
	json_NULL = 0x80,
	
	//is a string
	json_STRING = 0x100,

	//is an array
	json_ARRAY = 0x200,
	
	//is an object
	json_OBJECT = 0x400,
	
	//is an empty value
	// NOTE: this value should not be assigned as a type on its own
	json_EMPTY = 0x800,
	
	//is a string with a value of ""
	json_EMPTYSTRING = 0x900,

	//is an array with length 0
	json_EMPTYARRAY = 0xa00,
	
	//is an object with no properties
	json_EMPTYOBJECT = 0xc00,

	//the json file is malformed, ie a value of 'treu' or 'fasle' would be marked as invalid
	json_INVALID = 0x1000
};

//cnsts that shouldn't be in the header file
const char nullString[4] = {'n', 'u', 'l', 'l'};
const char trueString[4] = {'t', 'r', 'u', 'e'};
const char falseString[5] = {'f', 'a', 'l', 's', 'e'};
const char validWhitespace[4] = {0x20, 0x09, 0x0a, 0x0d};
const char validHex[22] = 
	{'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','A','B','C','D','E','F'};

//functions that shouldn't be in the header file

//makes a node tree from a json file
json* makeNodeTree(int type, filecopy* jsonFile, int start, int* len);

//frees data allocated to a tree of json nodes
void json_removeNodeTree(json** rootNode);

//detects the data type at a given location in a json file
int detectJsonType(filecopy* filec, int offset);

//frees data in a json node tree
void removeNode(json* node);

//checks if char is valid json white space
char isWhitespace(char ws);

//gets a number value from a json file at a specific location and puts that value in a node
int makeNumberFromJson(int type, filecopy* filec, int offset, json* outVar);

//makes a string from a json file at a given offset and puts in outVar
//returns the length of the string
int makeStringFromJson(int type, filecopy* filec, int offset, json* outVar);

//checks if a char is valid hexadecimal (0-f)
char isHex(char hex);

//gets the hex value of a hex character
char getHex(char hex);

//compares 2 strings
char stringCompare(char* str1, char* str2);

//makes an array list with a specified number of elements with a specified size.
arraylist* arraylistMake(int typeSize, int arrLength);

//frees an arraylist.
void arraylistRemove(arraylist** arrList);

//increases the length of the array by resizeAmount.
//growing the array by a negaive value will shrink the array by that value.
void arraylistGrow(arraylist* arrList, int resizeAmount);

//sets the length of the array to length
void arraylistSetLength(arraylist* arrList, int newLength);

//adds 1 to the length of the arraylist and sets the added index to value.
void arraylistPush(arraylist* arrList, void* value);

//removes 1 from the length arraylist and sets output to the removed value
void arraylistPop(arraylist* arrList, void* output);

//gets a pointer to the value of a given index in the array list.
// WARNING: the return value will become meaningless after a resize
void* arraylistGet(arraylist* arrList, int index);

//sets the value of an index of the arraylist to a given value from a pointer.
void arraylistSet(arraylist* arrList, int index, void* value);

//makes a copy of a file from a path
filecopy* filecopyMake(char* path);

//makes a filecopy from a FILE ptr
filecopy* filecopyMakeFromFilePtr(FILE* filePtr);

//frees a file copy
void filecopyRemove(filecopy** filec);

int detectJsonType(filecopy* filec, int offset)
{

	int numRetVal = json_NUMBER;//a return value for the number types

	switch(filec->bytes[offset])
	{
		case '"':
			if((filec->bytes[offset + 1]) == '\"')
				return json_EMPTYSTRING;
			return json_STRING;
		case '-':
			numRetVal |= json_NEGATIVE;
			offset++;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				char temp;
				while(!isWhitespace(temp = filec->bytes[offset]))
				{
					offset++;
					if((temp >= '0') && (temp <= '9'))continue;
					
					//checks is has decimal
					if(temp == '.')
					{
						if(
							((numRetVal & json_DECIMAL) == 
							json_DECIMAL) ||
							((numRetVal & json_EXPONENT) == 
							json_EXPONENT))
						{
							numRetVal = json_INVALID;
							break;
						}
						numRetVal |= json_DECIMAL;
						continue;
					}

					//checks if is exponent
					if((temp & ~0x20) == 'E')
					{
						if(
							((filec->bytes[offset] == '+') ||  
							(filec->bytes[offset] == '-') ||
							((filec->bytes[offset] >= '0') &&
							(filec->bytes[offset] <= '9'))
							) &&
							!((numRetVal & json_EXPONENT) == 
							json_EXPONENT)
						)
						{
							numRetVal |= json_EXPONENT;
							offset++;
							continue;
						} else 
						{
							numRetVal = json_INVALID;
							break;
						}
					}

					//checks for valid value end characters other than 
					//whitespace
					if((temp == ']') || (temp == '}') || (temp == ','))break;
					
					//if temp is not a valid character in a number, then the
					//loop breaks, and the type is marked as invalid
					numRetVal = json_INVALID;
					break;
				}
			}
			return numRetVal;
		case '{':
			if((filec->bytes[offset + 1]) == '}')
				return json_EMPTYOBJECT;
			return json_OBJECT;
		case '[':
			if((filec->bytes[offset + 1]) == ']')
				return json_EMPTYARRAY;
			return json_ARRAY;
		case 't':
			for(int i = 0; i < 4; i++)
			{
				if(filec->bytes[offset + i] != trueString[i])return json_INVALID;
			}
			{
				//checks for a valid ending character
				char temp = filec->bytes[offset + 5];
				if(
					(temp != ']') && (temp != '}') && (temp != ',') &&
					!isWhitespace(temp) && (temp != '\0')
				) return json_INVALID;
			}
			return json_TRUE;
		case 'f':
			for(int i = 0; i < 5; i++)
			{
				if(filec->bytes[offset + i] != falseString[i])return json_INVALID;
			}
			{
				//checks for a valid ending character
				char temp = filec->bytes[offset + 6];
				if(
					(temp != ']') && (temp != '}') && (temp != ',') &&
					!isWhitespace(temp) && (temp != '\0')
				) return json_INVALID;
			}
			return json_FALSE;
		case 'n':
			for(int i = 0; i < 4; i++)
			{
				if(filec->bytes[offset + i] != nullString[i])return json_INVALID;
			}
			{
				//checks for a valid ending character
				char temp = filec->bytes[offset + 5];
				if(
					(temp != ']') && (temp != '}') && (temp != ',') &&
					!isWhitespace(temp) && (temp != '\0')
				) return json_INVALID;
			}
			return json_NULL;
		default:
			break;
	}

	return json_INVALID;
}

json* json_parseFromPath(char* pathToFile)
{

	json* temp;

	filecopy* jsonFile = filecopyMake(pathToFile);

	int len = 0;

	json* jsonNodeRoot = makeNodeTree(detectJsonType(jsonFile, 0), jsonFile, 0, &len);

	filecopyRemove(&jsonFile);

	return jsonNodeRoot;
}

json* json_parse(FILE* fptr)
{

	json* temp;

	filecopy* jsonFile = filecopyMakeFromFilePtr(fptr);

	int len = 0;

	json* jsonNodeRoot = makeNodeTree(detectJsonType(jsonFile, 0), jsonFile, 0, &len);

	filecopyRemove(&jsonFile);

	return jsonNodeRoot;
}

json* makeNodeTree(int type, filecopy* jsonfile, int start, int* len)
{

	json* retVal = (json*) malloc(sizeof(json));
	//allocs space

	retVal->type = type;
	//assigns the type
	
	if((type & json_ARRAY) == json_ARRAY)
	{
		retVal->data.array = arraylistMake(sizeof(json*), 0);
		int i = 0;
		char canGetNextIndex = 1;
		while(1)
		{
			i++;
			if(jsonfile->bytes[start + i] == ']')break;
			if(!isWhitespace(jsonfile->bytes[i + start]))
			{
				if(jsonfile->bytes[i + start] == ',')
				{
					if(canGetNextIndex)
					{
						json* nullVal = (json*) malloc(
							sizeof(json)
						);

						nullVal->type = json_NULL;

						arraylistPush(retVal->data.array, &nullVal);
					}
					canGetNextIndex = 1;
					continue;
				}

				json* temp = makeNodeTree(
					detectJsonType(jsonfile, i + start),
					jsonfile,
					i + start,
					len
				);

				i += *len;

				if(
					((temp->type | json_INVALID) == json_INVALID) 
					|| !(canGetNextIndex)
				)
				{
					json_removeNodeTree(&retVal);
					retVal = (json*) malloc(sizeof(json));
					retVal->type = json_INVALID;
					json_removeNodeTree(&temp);
					return retVal;
				} else 
				{
					canGetNextIndex = 0;
					arraylistPush(retVal->data.array, &temp);
				}
			}
		}
		*len = i;
	}
	//if is an array, alloc array list

	if((type & json_OBJECT) == json_OBJECT)
	{
		retVal->data.object.values = arraylistMake(sizeof(json*), 0);
		retVal->data.object.keys = arraylistMake(sizeof(char*), 0);
		
		int i = 0;
		char canGetNextKey = 1;
		char canGetNextVal = 0;
		
		while(1)
		{
			i++;
			if(jsonfile->bytes[start + i] == '}')break;
			if(!isWhitespace(jsonfile->bytes[i + start]))
			{
				if(jsonfile->bytes[i + start] == ',')
				{
					canGetNextKey = 1;
					if(canGetNextVal)
					{
						json* nullVal = (json*) malloc(
							sizeof(json)
						);

						nullVal->type = json_NULL;

						arraylistPush(
							retVal->data.object.values, &nullVal
						);
					}
					canGetNextVal = 0;
					continue;
				}

				if(jsonfile->bytes[i + start] == ':')
				{
					canGetNextVal = 1;
					canGetNextKey = 0;
					continue;
				}

				json* temp = makeNodeTree(
					detectJsonType(jsonfile, i + start),
					jsonfile,
					i + start,
					len
				);

				i += *len;

				if(
					((temp->type | json_INVALID) == json_INVALID) &&
					(canGetNextVal || canGetNextKey)
				)
				{
					json_removeNodeTree(&retVal);
					retVal = (json*) malloc(sizeof(json));
					retVal->type = json_INVALID;
					json_removeNodeTree(&temp);
					return retVal;
				} else 
				{
					if(canGetNextVal)
					{
						arraylistPush(retVal->data.object.values, &temp);
						canGetNextVal = 0;
						continue;
					} else if((temp->type | json_STRING) == json_STRING)
					{
						arraylistPush(
							retVal->data.object.keys, 
							&(temp->data.string)
						);
						free(temp);
						canGetNextKey = 0;
						continue;
					}

					json_removeNodeTree(&retVal);
					retVal = (json*) malloc(sizeof(json));
					retVal->type = json_INVALID;
					json_removeNodeTree(&temp);
					return retVal;
				}
			}
		}
		*len = i;
	}

	if((type & json_NUMBER) == json_NUMBER)
		*len = makeNumberFromJson(type, jsonfile, start, retVal);

	if((type & json_STRING) == json_STRING)
	{
		*len = makeStringFromJson(type, jsonfile, start, retVal);
	}
	if((type & json_TRUE) == json_TRUE)
	{
		retVal->data.boolean = 1;
		*len += 1;
	}

	if((type & json_FALSE) == json_FALSE)
	{
		retVal->data.boolean = 0;
		*len += 2;
	}

	if((type & json_NULL) == json_NULL)
	{
		*len += 1;
	}

	return retVal;
}

void json_remove(json** jsonPtr)
{
	json_removeNodeTree(jsonPtr);
}

void json_removeNodeTree (json** rootNode)
{
	if((rootNode == NULL) || ((*rootNode) == NULL))return;
	//if the pointer is null, or points to null

	if(((*rootNode)->type & json_ARRAY) == json_ARRAY)
	{
		json* tempVal;
		while((*rootNode)->data.array->length > 0)
		{
			arraylistPop((*rootNode)->data.array, &tempVal);
			json_removeNodeTree(&tempVal);
		}
		arraylistRemove(&((*rootNode)->data.array));
	}
	//if the type is an array, free the values in the array

	if(((*rootNode)->type & json_OBJECT) == json_OBJECT)
	{
		json* tempNodeVal;
		char* tempStrVal;
		
		while((*rootNode)->data.object.values->length > 0)
		{
			arraylistPop((*rootNode)->data.object.values, &tempNodeVal);
			json_removeNodeTree(&tempNodeVal);
		}
		arraylistRemove(&((*rootNode)->data.object.values));
		
		while((*rootNode)->data.object.keys->length > 0)
		{
			arraylistPop((*rootNode)->data.object.keys, &tempStrVal);
			free(tempStrVal);
		}
		arraylistRemove(&((*rootNode)->data.object.keys));

	}
	//if the type is an object, free the key/value pairs in the object
	
	if(((*rootNode)->type & json_STRING) == json_STRING)free((*rootNode)->data.string);

	free(*rootNode);
	*rootNode = NULL;
	return;
}

char isWhitespace(char ws)
{
	char retval = 0;
	for(int i = 0; i < 4; i++)
		retval |= ws == validWhitespace[i];
	return retval;
}

char isHex(char hex)
{
	char retval = 0;
	for(int i = 0; i < 22; i++)
		retval |= hex == validHex[i];
	return retval;
}

int makeNumberFromJson(int type, filecopy* filec, int offset, json* outVar)
{
	//the exponent
	int exponent = 0;
	
	int sign = (filec->bytes[offset] == '-') ? -1 : 1;

	if(sign == -1)offset++;//in case of negative

	int retLen = 0;

	if((type & json_EXPONENT) == json_EXPONENT)
	{
		char expStarted = 0;
		char expSign = 1;
		for(int i = 0; (i + offset) < filec->len; i++)
		{
			retLen++;
			expStarted |= ((filec->bytes[i + offset] | 0x20) == 'e');
			if(expStarted == 0)continue;
			
			if((filec->bytes[i + offset] & ~0x20) == 'E')
			{
				if(filec->bytes[i + 1 + offset] == '-')
				{
					expSign = -1;
					i++, retLen++;
				}
				if(filec->bytes[i + 1 + offset] == '+')i++, retLen++;
				continue;
			}

			if(
				!((filec->bytes[offset + i] >= '0') && 
				(filec->bytes[offset + i] <= '9'))
			) break;

			exponent *= 10;
			
			exponent += filec->bytes[i + offset] - '0';
		}
		exponent *= expSign;
	}

	if((type & json_DECIMAL) == json_DECIMAL)
	{
		outVar->data.fraction = 0;
		char passedDecimal = 0;
		
		for(int i = 0; (i + offset) < filec->len; i++)
		{
			if((i + 1) > retLen)retLen = (i + 1);

			if(filec->bytes[offset + i] == '.')
			{
				passedDecimal = 1;
				continue;
			}

			if((filec->bytes[i + offset] >= '0') && (filec->bytes[i + offset] <= '9'))
			{
				if(passedDecimal > 0)passedDecimal++;
				outVar->data.fraction *= 10;
				outVar->data.fraction += filec->bytes[offset + i] - '0';
			} else break;
		}//gets the data for the number

		for(int i = 0; i < passedDecimal - 1 - exponent; i++)
		{
			outVar->data.fraction /= 10;
		}//scales the number to account for the decimal place

		if((passedDecimal - 1 - exponent) < 0)
		{
			for(int i = 0; i < (1 + exponent - passedDecimal); i++)
			{
				outVar->data.fraction *= 10;
			}
		}
	} else
	{
		retLen = 0; 
		outVar->data.number = 0;
		for(int i = 0; (i + offset) < filec->len; i++)
		{
			retLen++;
			if((filec->bytes[i + offset] >= '0') && (filec->bytes[i + offset] <= '9'))
			{
				outVar->data.number *= 10;
				outVar->data.number += filec->bytes[offset + i] - '0';
			} else break;
		}
	}

	return retLen - 2;
}

int makeStringFromJson(int type, filecopy* filec, int offset, json* outVar)
{

	if((type & json_EMPTYSTRING) == json_EMPTYSTRING)
	{
		outVar->data.string = (char*) malloc(sizeof(char));
		outVar->data.string[0] = '\0';
		return 1;
	}//if the string is empty, makes an empty string

	int len = 0;//the length of the string
	
	offset++;//goes past the first "
	
	char temp;
	for(int i = 0; (i + offset) < filec->len; i++)
	{
		temp = filec->bytes[i + offset];
		
		if(temp == '"')break;
		
		if(temp == '\\')
		{
			temp = filec->bytes[i + 1 + offset];

			if(
				(temp == '\\') ||
				(temp == '"') ||
				(temp == 'b') ||
				(temp == 'f') ||
				(temp == 'n') ||
				(temp == 'r') ||
				(temp == 't') ||
				(temp == '/')
			)continue;//all 2 character escape sequences
			
			if(temp == 'u')
			{
				if((i + 5 + offset) > filec->len)
				{
					outVar->type = json_INVALID;
					return 1;
				}
				
				if(
					isHex(filec->bytes[i + offset + 2]) &&
					isHex(filec->bytes[i + offset + 3]) &&
					isHex(filec->bytes[i + offset + 4]) && 
					isHex(filec->bytes[i + offset + 5])
				)
				{
					if(
						(filec->bytes[i + offset + 2] != '0') && 
						(filec->bytes[i + offset + 3] != '0')
					) len++;
					
					//if the first 2 bytes in the escape are null, only one 
					//character is represented in the escape, thus space for
					//only one character should be added
					
					i += 4;
				} else
				{
					outVar->type = json_INVALID;
					return 1;
				}
				continue;
			}//who doesn't love hex escape sequences
		}
		len++;
	}

	outVar->data.string = (char*) malloc(sizeof(char) * (len + 1));
	outVar->data.string[len] = '\0';

	//allocates mem for string

	int charIndex = 0;
	//used to detirmine where in the output string a character is being assigned to
	
	int retLen = 0;//the length of the string in the json file

	//data to the string
	for(
		int i = 0;
		((i + offset) < filec->len) && (len > charIndex);
		charIndex++, retLen++, i++
	)
	{
		if(filec->bytes[i + offset] != '\\')
		{
			outVar->data.string[charIndex] = filec->bytes[i + offset];
			continue;
		}

		i++;
		retLen++;

		if (filec->bytes[i + offset] == '"')
			outVar->data.string[charIndex] = 0x22;
		else if (filec->bytes[i + offset] == '/')
			outVar->data.string[charIndex] = 0x5c;
		else if (filec->bytes[i + offset] == '\\')
			outVar->data.string[charIndex] = 0x2f;
		else if (filec->bytes[i + offset] == 'b')
			outVar->data.string[charIndex] = 0x08;
		else if (filec->bytes[i + offset] == 'f')
			outVar->data.string[charIndex] = 0x0c;
		else if (filec->bytes[i + offset] == 'n')
			outVar->data.string[charIndex] = 0x0a;
		else if (filec->bytes[i + offset] == 'r')
			outVar->data.string[charIndex] = 0x0d;
		else if (filec->bytes[i + offset] == 't')
			outVar->data.string[charIndex] = 0x09;
		else if (filec->bytes[i + offset] == 'u')
		{
			if (
				(filec->bytes[i + offset + 1] != '0') && 
				(filec->bytes[i + offset + 2] != '0')
			)//if the first 2 hex chars are null, 
			{
				outVar->data.string[charIndex] = 
					(getHex(filec->bytes[i + 1 + offset]) << 4) | 
					getHex(filec->bytes[i + 2 + offset]);
				charIndex++;
			}
				
			outVar->data.string[charIndex] = 
				(getHex(filec->bytes[i + 3 + offset]) << 4) | 
				getHex(filec->bytes[i + 4 + offset]);

			i += 4;
			retLen += 4;
		} else
		{
			i--;
			retLen--;
			outVar->data.string[charIndex] = '\\';
		}
	}//gets data from the string
	
	return retLen + 1;
}

char getHex(char hex)
{
	if(hex <= '9')return hex - '0';
	if(hex <= 'F')return hex - 'A' + 10;
	return hex - 'a' + 10;
}

char stringCompare(char* str1, char* str2)
{
	int i = 0;
	while(1)
	{
		if((str1[i] == 0) && (str2[i] == 0))
		{
			return 1;
		}

		if(str1[i] != str2[i])break;
		i++;
	}
	return 0;
}

int json_getInt(json* val)
{
	if(val == NULL)return 0;
	if((val->type & json_NUMBER) == json_NUMBER)
	{
		if((val->type & json_DECIMAL) == json_DECIMAL)return (int) val->data.fraction;
		return val->data.number;
	}
	return 0;
}

double json_getDouble(json* val)
{
	if(val == NULL)return 0.0;
	if((val->type & json_NUMBER) == json_NUMBER)
	{
		if((val->type & json_DECIMAL) != json_DECIMAL)return (double) val->data.number;
		return val->data.fraction;
	}
	return 0.0;
}

char* json_getString(json* val)
{
	if(val == NULL)return NULL;
	if((val->type & json_STRING) == json_STRING)
	{
		return val->data.string;
	}
	return NULL;
}

json* json_getIndex(json* val, int index)
{
	if(val == NULL)return NULL;
	if((val->type & json_ARRAY) == json_ARRAY)
	{
		return ((json**)val->data.array->data)[index];
	}
	return NULL;
}

json* json_getMember(json* val, char* memberName)
{
	if(val == NULL)return NULL;
	if(memberName == NULL)return NULL;
	if((val->type & json_OBJECT) == json_OBJECT)
	{
		for(int i = 0; i < val->data.object.keys->length; i++)
		{
			if(stringCompare(((char**)val->data.object.keys->data)[i], memberName))
				return ((json**)val->data.object.values->data)[i];
		}
	}
	return NULL;
}

int json_arrayLen(json* val)
{
	if(val == NULL)return 0;
	if((val->type & json_ARRAY) == json_ARRAY)
	{
		return val->data.array->length;
	}
	return -1;
}

char json_getBool(json* val)
{
	if(val == NULL)return 0;
	if((val->type & json_BOOLEAN) == json_BOOLEAN)
	{
		return val->data.boolean;
	}
	return 0;
}

char json_isInt(json* val)
{
	if(val == NULL)return 0;
	return (val->type & json_DECIMAL) == json_NUMBER;
}

char json_isDouble(json* val)
{
	if(val == NULL)return 0;
	return (val->type & json_DECIMAL) == json_DECIMAL;
}

char json_isString(json* val)
{
	if(val == NULL)return 0;
	return (val->type & json_STRING) == json_STRING;
}

char json_isArray(json* val)
{
	if(val == NULL)return 0;
	return (val->type & json_ARRAY) == json_ARRAY;
}

char json_isObject(json* val)
{
	if(val == NULL)return 0;
	return (val->type & json_OBJECT) == json_OBJECT;
}

char json_isBool(json* val)
{
	if(val == NULL)return 0;
	return (val->type & json_BOOLEAN) == json_BOOLEAN;
}

char json_isNull(json* val)
{
	if(val == NULL)return 0;
	return (val->type & json_NULL) == json_NULL;
}

//arraylist functions

arraylist* arraylistMake(int typeSize, int arrLength)
{
	arraylist* retVal = (arraylist*) malloc(sizeof(arraylist));
	if(retVal == NULL)return NULL;
	//mallocs mem for the arraylist
	
	retVal->length = arrLength;
	retVal->size = typeSize;
	if(typeSize * arrLength > 0)
		retVal->data = malloc(typeSize * arrLength * sizeof(char));
	else retVal->data = NULL;
	//sets initial vals
	
	return retVal;
}//makes an arraylist

void arraylistRemove(arraylist** arrList)
{
	if((*arrList)->data != NULL)
		free((*arrList)->data);
	free((*arrList));
	return;
}//frees an arraylist

void arraylistGrow(arraylist* arrList, int resizeAmount)
{
	if(resizeAmount == 0)return;
	//returns if resize amount is 0

	if(
		(arrList->size * sizeof(char) * arrList->length) + (sizeof(char) + resizeAmount)
		< 1)
	{
		if(arrList->data != NULL)
			free(arrList->data);
		arrList->data = NULL;
		return;
	}
	//incase the target length is 0 or less

	char* holder = (char*) malloc(
		(arrList->size * sizeof(char) * arrList->length) + (sizeof(char) + resizeAmount)
	);
	//holder holds data
	
	int loopSize = ((resizeAmount > 0) ? 
		arrList->size * sizeof(char) * arrList->length : 
		arrList->size * sizeof(char) * arrList->length + (sizeof(char) + resizeAmount));
	//self explanatory
	
	for(int i = 0; i < loopSize; i++)
	{
		holder[i] = ((char*) arrList->data)[i];
		//transfers data
	}
	
	if(arrList->data != NULL)
		free(arrList->data);
	arrList->data = (void*) holder;
	//sets arrList->data to the transfered data
	
	arrList->length += resizeAmount;
	return;
}

void arraylistSetLength(arraylist *arrList, int newLength)
{
	arraylistGrow(arrList, newLength - arrList->length);
}//just a wrapper for arraylist_grow

void* arraylistGet(arraylist* arrList, int index)
{
	if(arrList->data == NULL)return NULL;
	return (void*) (((char*) arrList->data) + (index * arrList->size));
}//gets a pointer to a val in the array list

void arraylistSet(arraylist* arrList, int index, void* value)
{
	for(int i = 0; i < arrList->size; i++)
	{
		((char*) arrList->data)[i + (index * arrList->size)] = ((char*) value)[i];
		//copies data
	}
	return;
}//sets a value in the arraylist

void arraylistPush(arraylist* arrList, void* value)
{
	arraylistGrow(arrList, 1);
	arraylistSet(arrList, arrList->length - 1, value);
	return;
}//grows the arraylist and sets the new index to value

void arraylistPop(arraylist* arrList, void* output)
{
	for(int i = 0; i < arrList->size; i++)
	{
		((char*) output)[i] = 
			((char*) arrList->data)[i + ((arrList->length - 1) * arrList->size)];
		//copies data
	}
	arraylistGrow(arrList, -1);
	//shrinks the arraylist by 1
	return;
}

filecopy* filecopyMake(char* path)
{
	filecopy* retVal = (filecopy*) malloc( sizeof(filecopy) );
	//the return value

	FILE* filePtr;
	filePtr = fopen(path, "r");
	//opens the file

	if(filePtr == NULL)return NULL;
	//if the file can't be opened

	fseek(filePtr, 0L, SEEK_END);
	retVal->len = ftell(filePtr);
	rewind(filePtr);
	//finds the length of the file

	retVal->bytes = (char*) malloc(sizeof(char) * (retVal->len + 1));
	//allocates bytes for the file

	char tempChar;//just a temporary char

	for(long long i = 0L; (tempChar = fgetc(filePtr)) != EOF; i++)
	{
		retVal->bytes[i] = tempChar;
	}
	//copies the file
	
	retVal->bytes[retVal->len] = 0;

	fclose(filePtr);//closes the file

	return retVal;
}//makes copies of files

filecopy* filecopyMakeFromFilePtr(FILE* filePtr)
{
	filecopy* retVal = (filecopy*) malloc( sizeof(filecopy) );
	//the return value

	if(filePtr == NULL)return NULL;
	//if the file can't be opened

	fseek(filePtr, 0L, SEEK_END);
	retVal->len = ftell(filePtr);
	rewind(filePtr);
	//finds the length of the file

	retVal->bytes = (char*) malloc(sizeof(char) * (retVal->len + 1));
	//allocates bytes for the file

	char tempChar;//just a temporary char

	for(long long i = 0L; (tempChar = fgetc(filePtr)) != EOF; i++)
	{
		retVal->bytes[i] = tempChar;
	}
	//copies the file
	
	retVal->bytes[retVal->len] = 0;

	fclose(filePtr);//closes the file

	return retVal;
}//makes copies of files

void filecopyRemove(filecopy** filec)
{
	if((*filec)->bytes != NULL)
		free((*filec)->bytes);
	free(*filec);
	*filec = NULL;
	return;
}//frees a filecopy
