/*-------- HEADERS --------*/

#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

/*-------- MACROS --------*/

#define loop for(;;)
#define fn auto
#define let auto
#define then

#define CA do { /* check above */ \
	if(sp >= 10) then puts("stack overflow"), exit(-1);}\
	while(0)
#define CU(n) do { /* check under */ \
	if(sp < n) then puts("stack underflow"), exit(-2);}\
	while(0)

/*-------- DATA --------*/

/* most recently read chars */
static char buf[80];
static char *bp;

/* last parsed word */
static char word[8];

/* last parsed number */
static uint32_t number;

/* if state */
static int ifdepth = 0;
static uint64_t ifstate = 0;

/* number stack */
static uint32_t stack[11];
static int sp = 0; // stack pointer

/* dictionary */
struct dictionary {
	uint32_t name;
	uint32_t place;
};
static dictionary words[100] = {0};
static uint32_t wp = 0;
static uint32_t dict[400] = {0};
static uint32_t dp = 0;

/*-------- CODE --------*/

/*-- auxiliary --*/

static constexpr
fn hash (const char *str) -> uint32_t
{ /* murmur */
	let hash = 3323198485ul;
	for(; *str; ++str) {
		hash ^= *str;
		hash *= 0x5bd1e995;
		hash ^= hash >> 15;
	}
	return hash << 1;
}

static inline
fn powten (unsigned n) -> int
{
	let x = 1;
	for(; n; n--)
		x *= 10;
	return x;
}

static inline
fn findword (uint32_t word) -> uint32_t
{
	for(let i = 0u; i < wp; i++)
		if(words[i].name == word) return words[i].place;

	puts("undefined word"), exit(-5);
}

/*-- builtins --*/

/*- stack -*/
static inline
fn push (int n) -> void
{
	CA;
	stack[++sp] = n;
}
static inline
fn pop (void) -> int
{
	CU(1);
	return stack[sp--];
}
static inline
fn peek (void) -> int
{
	CU(1);
	return stack[sp];
}
static inline
fn swap (void) -> void
{
	CU(2);
	let temp = stack[sp];

	stack[sp]     = stack[sp - 1];
	stack[sp - 1] = temp;
}
static inline
fn rot (void) -> void
{
	CU(3);
	let temp = stack[sp];

	stack[sp]     = stack[sp - 1];
	stack[sp - 1] = stack[sp - 2];
	stack[sp - 2] = temp;
}
static inline
fn drop (void) -> void
{
	CU(1);
	sp--;
}
static inline
fn nip (void) -> void
{
	CU(2);
	stack[sp - 1] = stack[sp];
	sp--;
}

/*- arithmetic -*/
static inline
fn add (void) -> void
{
	CU(2);
	stack[sp - 1] += stack[sp];
	sp--;
}
static inline
fn sub (void) -> void
{
	CU(2);
	stack[sp - 1] -= stack[sp];
	sp--;
}
static inline
fn mul (void) -> void
{
	CU(2);
	stack[sp - 1] *= stack[sp];
	sp--;
}
static inline
fn div (void) -> void
{
	CU(2);
	stack[sp - 1] /= stack[sp];
	sp--;
}

/*- logic -*/
static inline
fn greater (void) -> void
{
	CU(2);
	let first  = stack[sp - 1];
	let second = stack[sp];
	stack[--sp] = (first > second);
}
static inline
fn less (void) -> void
{
	CU(2);
	let first  = stack[sp - 1];
	let second = stack[sp];
	stack[--sp] = (first < second);
}
static inline
fn equal (void) -> void
{
	CU(2);
	let first  = stack[sp - 1];
	let second = stack[sp];
	stack[--sp] = (first == second);
}
static inline
fn iszero (void) -> void
{
	CU(1);
	stack[sp] = (stack[sp] == 0);
}

/*- conditionals -*/
static inline
fn ifword (void) -> void
{
	ifstate ^= ( pop() ? 1 : 0 ) << ifdepth++;
}

/*- colon -*/
fn readword (void) -> void;
fn readnum  (void) -> void;

fn colon (void) -> void
{
	assert(wp < 100);

	readword();
	words[wp++] = { hash(word), dp };

	while(bp < buf + 80) {
		assert(dp < 400);

		if(isspace(*bp)) {
			bp++;
			continue;
		} else if(isdigit(*bp)) {
			readnum();
			dict[dp++] = (number << 1) + 1;
			continue;
		}
		readword();
		dict[dp++] = hash(word);
		if(word[0] == ';') then break;
	}
	if(word[0] != ';')
		puts("malformed col def"), exit(-7);
}

/*- misc -*/
static inline
fn list (void) -> void
{
	if (sp <= 0) then return;

	for(let i = 1; i < sp; i++)
		printf("%d,", stack[i]);

	printf("%d.\n", stack[sp]);
}

static inline
fn quote (void) -> void
{
	readword();
	push(hash(word));
}

fn evalword (uint32_t) -> void;

static inline
fn unquote (void) -> void
{
	evalword(pop());
}

/*-- parser --*/

fn readnum (void) -> void
{
	char n[10] = {0};
	let i = 0;
	for(; i < 10; i++) {
		if(!isdigit(*bp)) break;
		n[i] = *bp++;
	}
	i--;

	let num = 0;
	for(let j = 0; i >= 0; i--, j++)
		num += powten(i) * (n[j] - '0');

	number = num;
}

fn readword (void) -> void
{
	memset(word, 0, 8);
	for(let i = 0; i < 8; i++) {
		if(!isgraph(*bp)) break;
		word[i] = *bp++;
	}
	while(isgraph(*bp++));
}

/*-- evaluator --*/

fn evalcol (uint32_t cword) -> void;

fn evalword (uint32_t w) -> void
{
	if(ifdepth) {
		let flagbit = 1ul << (ifdepth - 1);
		let executable = ifstate & flagbit;

		if(w == hash("then")) {
			ifstate &= flagbit - 1;
			ifdepth--;
			return;
		} else if(not executable) {
			if(w == hash("else")) then ifstate ^= flagbit;
			return;
		} else if(executable && w == hash("else"))
			ifstate ^= flagbit;
	}

	if(1 & w) {
		push(w >> 1);
		return;
	}

	switch(w) {
	case(hash("dup")):
		push(peek());
		return;
	case(hash("swap")):
		swap();
		return;
	case(hash("rot")):
		rot();
		return;
	case(hash("drop")):
		drop();
		return;
	case(hash("nip")):
		nip();
		return;
	case(hash(".")):
		printf("%d\n", pop());
		return;
	case(hash("+")):
		add();
		return;
	case(hash("-")):
		sub();
		return;
	case(hash("*")):
		mul();
		return;
	case(hash("/")):
		div();
		return;
	case(hash(">")):
		greater();
		return;
	case(hash("<")):
		less();
		return;
	case(hash("=")):
		equal();
		return;
	case(hash("0=")):
		iszero();
		return;
	case(hash(":")):
		colon();
		return;
	case(hash(";")):
		return;
	case(hash("if")):
		ifword();
		return;
	case(hash("else")):
		return;
	case(hash("then")):
		return;
	case(hash("'")):
		quote();
		return;
	case(hash("eval")):
		unquote();
		return;
	case(hash(".s")):
		list();
		return;
	case(hash("exit")):
		exit(0);
		return;
	default:
		evalcol(w);
		return;
	}
}

fn evalcol (uint32_t cword) -> void
{
	let p = findword(cword);

	while(dict[p+1] != hash(";")) {
		evalword(dict[p++]);
	}
	return evalword(dict[p]);
}

fn eval (void) -> void
{
	bp = buf;
	while(*bp != 0) {
		if(isspace(*bp)) {
			bp++;
		} else if(isdigit(*bp)) {
			readnum();
			if(ifdepth == 0 || ifstate & (1 << (ifdepth - 1)))
				push(number);
		} else {
			readword();
			evalword(hash(word));
		}
	}
}

/*-- main --*/

fn main () -> int
{
	loop {
		/* prompt */
		write(1, "→ ", 4);

		/* read */
		memset(buf, 0, 80);
		read(0, buf, 79);

		/* eval */
		eval();
	}
	return 0;
}
