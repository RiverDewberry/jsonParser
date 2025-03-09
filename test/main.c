#include <stdio.h>
#include "../jsonParser.h"

int main(void)
{
	json* test = json_parseFromPath("./test.json");

	printf("test 1 -> %s\n", json_getString(json_getMember(test, "test1")));

	printf("test 2 -> %d\n", 
		json_getInt(json_getMember(test, "test2"))
	);
	
	printf("test 3 -> %lf\n", 
		json_getDouble(json_getMember(test, "test3"))
	);
	
	printf("test 4 -> %d\n", 
		json_getBool(json_getMember(test, "test4"))
	);
	
	printf("test 5 -> %d\n", 
		json_isNull(json_getMember(test, "test5"))
	);

	json *testArr = json_getMember(test, "test6");

	printf("test 6 -> %d, %d, %d, %d\n", 
		json_getInt(json_getIndex(testArr, 0)),
		json_getInt(json_getIndex(testArr, 1)),
		json_getInt(json_getIndex(testArr, 2)),
		json_getInt(json_getIndex(testArr, 3))
	);
	
	return 0;
}
