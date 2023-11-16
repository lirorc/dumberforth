#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cstdlib>

#define fn   auto
#define let  auto
#define then
#define loop for(;;)

template<class T>
struct Stack {
	T*   head;
	size_t size;
	size_t capacity;
};

