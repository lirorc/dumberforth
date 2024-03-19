# dumberForth
An ugly and incomplete forth-like interpreter in c++

You can define a word like so `: add1 1 + ;`

`2 add1` will remove the 2 from the stack and put 3.

Use .s instead of .S , dup instead of DUP and so on.

If you need a loop, you have to use recursion.

`: sum swap dup 0 = if drop else swap + sum then ;`

`0 12 23 34 45 sum` should consume those numbers and put 114 to stack

`: fact dup 0 = if drop else dup rot * swap 1 - fact then ;`

`: fac 1 swap fact ;`

`5 fac` should consume 5 and leave 120

You can quote words as in `4 2 ' + ' - nip eval`

Q: Why forth

A: Because it's simple

Q: Why C++

A: Because it's got constexpr (unlike C, my beloved)
