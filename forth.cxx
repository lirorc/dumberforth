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
static char buf[80]; // most recent read input
static char* bp;

/* last parsed word */
static char word[8]; // most recent read word

/* last parsed number */
static uint32_t number; // most recent read word

/* if state */
static int ifstate = 0;

/* number stack */
static uint32_t stack[11]; // number stack
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
fn hashdjb(char* str) -> uint32_t
{ /* djb2 */
	let hash = 5381ul;
	let c = 0;
	while(c = *str++)
		hash = ((hash << 5) + hash) + c;
	return hash;
}

static constexpr
fn hash(const char* str) -> uint32_t
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
fn powten(int n) -> int
{
	let x = 1;
	for(let i = 0; i < n; i++)
		x *= 10;
	return x;
}

/*-- builtins --*/

/*- stack -*/
static inline
fn push(int n) -> void
{
	CA;
	stack[++sp] = n;
}
static inline
fn pop() -> int
{
	return stack[sp--];
}
static inline
fn peek() -> int
{
	return stack[sp];
}
static inline
fn swap() -> void
{
	let temp = stack[sp];
	stack[sp] = stack[sp - 1];
	stack[sp - 1] = temp;
}
static inline
fn rot() -> void
{
	let temp = stack[sp];
	stack[sp] = stack[sp - 1];
	stack[sp - 1] = stack[sp - 2];
	stack[sp - 2] = temp;
}

/*- arithmetic -*/
static inline
fn add() -> void
{
	stack[sp - 1] += stack[sp];
	sp--;
}
static inline
fn sub() -> void
{
	stack[sp - 1] -= stack[sp];
	sp--;
}
static inline
fn mul() -> void
{
	stack[sp - 1] *= stack[sp];
	sp--;
}
static inline
fn div() -> void
{
	stack[sp - 1] /= stack[sp];
	sp--;
}

/*- logic -*/
static inline
fn greater() -> void
{
	let first = stack[sp - 1];
	let second = stack[sp];
	sp--;
	stack[sp] = first > second ? -1 : 0;
}
static inline
fn less() -> void
{
	let first = stack[sp - 1];
	let second = stack[sp];
	sp--;
	stack[sp] = first < second ? -1 : 0;
}
static inline
fn equal() -> void
{
	let first = stack[sp - 1];
	let second = stack[sp];
	sp--;
	stack[sp] = first == second ? -1 : 0;
}

/*- conditionals -*/
static inline
fn ifword() -> void
{
	ifstate = stack[sp--] ? 1 : 2;
}

/*- colon -*/
fn readword() -> void;
fn readnum() -> void;

fn colon() -> void
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
fn list() -> void
{
	if (sp <= 0) then return;

	for(let i = 1; i < sp; i++) {
		printf("%d ", stack[i]);
	}
	printf("%d\n", stack[sp]);
}

/*-- parser --*/

fn readnum() -> void
{
	char n[8] = {0};
	let i = 0;
	for(; i < 8; i++) {
		if(!isdigit(*bp)) then break;
		n[i] = *bp++;
	}
	i--;

	let num = 0;
	for(let j = 0; i >= 0; i--, j++)
		num += powten(i) * (n[j] - '0');

	number = num;
}

fn readword() -> void
{
	memset(word, 0, 8);
	for(let i = 0; i < 8; i++) {
		if(!isgraph(*bp)) break;
		word[i] = *bp++;
	}
	while(isgraph(*bp++));
}

/*-- evaluator --*/

fn evalcol(uint32_t cword) -> void;

fn evalword(uint32_t w) -> void
{
	switch(ifstate)
	{
	case 0: // no if
		break;
	case 1: // if true
		if(w == hash("else")) {
			ifstate = 3;
			return;
		}
		break;
	case 2: // if false
		if(w == hash("else") || w == hash("then")) {
			ifstate = 0;
		}
		return;
	default:
		if(w == hash("then")) {
			ifstate = 0;
		}
		return;
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

static inline
fn findword(uint32_t word) -> uint32_t
{
	for(let i = 0; i < wp; i++)
		if(words[i].name == word) return words[i].place;

	puts("undefined word"), exit(-5);
}
fn evalcol(uint32_t cword) -> void
{
	let p = findword(cword);
	while(dict[p] != hash(";")) {
		if(dict[p] == hash("number")) {
			push(dict[++p]);
			p++;
			continue;
		}
		evalword(dict[p++]);
	}
}

fn eval() -> void
{
	bp = buf;
	while(*bp != 0 && bp < buf + 80) {
		if(isspace(*bp)) {
			bp++;
			continue;
		} 
		if(isdigit(*bp)) {
			readnum();
			push(number);
			continue;
		}
		readword();
		evalword(hash(word));
	}
}

/*-- main --*/

fn main() -> int
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

