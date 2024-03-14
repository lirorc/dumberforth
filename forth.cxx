/*-------- HEADERS --------*/

#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/*-------- MACROS --------*/

#define loop for(;;)
#define fn auto
#define let auto
#define then
#define CA do {if(sp >= 10) then puts("stack overflow"), exit(-1);} while(0) /* check above */
#define CU(n) do {if(sp < n) then puts("stack underflow"), exit(-2);} while(0) /* check under */

/*-------- DATA --------*/

/* most recently read chars */
static char buf[80];
static char* bp;

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
fn hash (const char* str) -> uint32_t
{ /* murmur */
	let hash = 3323198485ul;
	for(; *str; ++str) {
		hash ^= *str;
		hash *= 0x5bd1e995;
		hash ^= hash >> 15;
	}
	return hash;
}

static inline
fn powten (int n) -> int
{
	let x = 1;
	for(let i = 0; i < n; i++)
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
	return stack[sp--];
}
static inline
fn peek (void) -> int
{
	return stack[sp];
}
static inline
fn swap (void) -> void
{
	let temp = stack[sp];
	stack[sp] = stack[sp - 1];
	stack[sp - 1] = temp;
}
static inline
fn rot (void) -> void
{
	let temp = stack[sp];
	stack[sp] = stack[sp - 1];
	stack[sp - 1] = stack[sp - 2];
	stack[sp - 2] = temp;
}
static inline
fn drop (void) -> void
{
	sp--;
}

/*- arithmetic -*/
static inline
fn add (void) -> void
{
	stack[sp - 1] += stack[sp];
	sp--;
}
static inline
fn sub (void) -> void
{
	stack[sp - 1] -= stack[sp];
	sp--;
}
static inline
fn mul (void) -> void
{
	stack[sp - 1] *= stack[sp];
	sp--;
}
static inline
fn div (void) -> void
{
	stack[sp - 1] /= stack[sp];
	sp--;
}

/*- logic -*/
static inline
fn greater (void) -> void
{
	let first = stack[sp - 1];
	let second = stack[sp];
	stack[--sp] = (first > second);
}
static inline
fn less (void) -> void
{
	let first = stack[sp - 1];
	let second = stack[sp];
	stack[--sp] = (first < second);
}
static inline
fn equal (void) -> void
{
	let first = stack[sp - 1];
	let second = stack[sp];
	stack[--sp] = (first == second);
}

/*- conditionals -*/
static inline
fn ifword (void) -> void
{
	if(ifdepth >= 64)
		puts("if depth reached"), exit(6);

	ifstate ^= (stack[sp--] ? 1 : 0) << ifdepth;
	ifdepth++;
}

/*- colon -*/
fn readword (void) -> void;
fn readnum  (void) -> void;

fn colon (void) -> void
{
	readword();
	words[wp++] = { hash(word), dp };

	while(bp < buf + 80) {
		if(isspace(*bp)) {
			bp++;
			continue;
		} else if(isdigit(*bp)) {
			readnum();
			dict[dp++] = hash("number");
			dict[dp++] = number;
			continue;
		}
		readword();
		dict[dp++] = hash(word);
		if(word[0] == ';') then break;
	}
}

/*- graphics -*/

/*- misc -*/
static inline
fn list (void) -> void
{
	if (sp <= 0) then return;

	printf("<");
	for(let i = 1; i < sp; i++)
		printf("%d,", stack[i]);

	printf("%d>\n", stack[sp]);
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
		let flagbit = 1 << (ifdepth - 1);
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

	switch(w)
	{
	case(hash("dup")):
		CU(1);
		push(peek());
		break;
	case(hash("swap")):
		CU(2);
		swap();
		break;
	case(hash("rot")):
		CU(3);
		rot();
		break;
	case(hash("drop")):
		CU(1);
		drop();
		break;
	case(hash(".")):
		CU(1);
		printf("%d\n", pop());
		break;
	case(hash("+")):
		CU(2);
		add();
		break;
	case(hash("-")):
		CU(2);
		sub();
		break;
	case(hash("*")):
		CU(2);
		mul();
		break;
	case(hash("/")):
		CU(2);
		div();
		break;
	case(hash(">")):
		CU(2);
		greater();
		break;
	case(hash("<")):
		CU(2);
		less();
		break;
	case(hash("=")):
		CU(2);
		equal();
		break;
	case(hash(":")):
		colon();
		break;
	case(hash(";")):
		break;
	case(hash("if")):
		CU(1);
		ifword();
		break;
	case(hash("else")):
		break;
	case(hash("then")):
		break;
	case(hash("'")):
		quote();
		break;
	case(hash("eval")):
		unquote();
		break;
	case(hash(".s")):
		list();
		break;
	case(hash("window")):
		break;
	case(hash("exit")):
		exit(0);
		break;
	default:
		evalcol(w);
		break;
	}
}

fn evalcol (uint32_t cword) -> void
{
	let p = findword(cword);
	while(dict[p] != hash(";")) {
		if(dict[p] == hash("number")) {
			p++;
			push(dict[p++]);
			continue;
		}
		evalword(dict[p++]);
	}
}

fn eval (void) -> void
{
	bp = buf;
	while(*bp != 0 && bp < buf + 80) {
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

