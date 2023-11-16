#include "forth.hxx"

Stack<int> nstack;

template<class T>
fn initstack(Stack<T>* s, size_t capacity = 100) -> void
{
	s->head = (T*) malloc(capacity * sizeof(T));
	if(s->head) then s->capacity = capacity;
}

template<class T>
fn freestack(Stack<T>* s) -> void
{
	free(s->head);
}

template<class T>
fn push(Stack<T>* s, T n) -> void
{
	if(s->capacity <= s->size) {
		s->head = (T*) realloc(s->head, (s->capacity *= 2) * sizeof(T));
	}
	*(s->head + s->size++) = n;
}

template<class T>
fn pop(Stack<T>* s) -> T
{
	return *(s->head + --s->size);
}

fn evalnum(char* buf) -> void
{
	let n = 0;
	sscanf(buf, "%d", &n);
	push(&nstack, n);
}

fn evalword(char* buf) -> void
{
	char word[8] {0};
	sscanf(buf, "%7s", word);
}

fn eval(char* buf, size_t size) -> void
{
	let b = buf;
	if(isdigit(*b)) {
		evalnum(b);
	} else if(isalpha(*b)) {
		evalword(b);
	}
	while((b < buf + size) && !isalnum(*(++b)));
}

fn main() -> int
{
	char buf[40];
	initstack(&nstack);
	loop {
		memset(buf, 0, 40);
		read(0, buf, 40);
		eval(buf, 40);
	}
	freestack(&nstack);
	return 0;
}
