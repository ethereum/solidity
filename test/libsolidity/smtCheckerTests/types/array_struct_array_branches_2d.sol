pragma experimental SMTChecker;

contract C
{
	struct S { uint[][] a; }
	function f(bool b) public pure {
		S[] memory c;
		c[0].a[0][0] = 0;
		if (b)
			c[0].a[0][0] = 1;
		else
			c[0].a[0][0] = 2;
		assert(c[0].a[0][0] > 0);
	}
}
// ----
// Warning: (124-130): Assertion checker does not yet support this expression.
// Warning: (124-128): Assertion checker does not yet implement type struct C.S memory
// Warning: (124-133): Assertion checker does not yet implement this expression.
// Warning: (124-136): Assertion checker does not yet implement this expression.
// Warning: (154-160): Assertion checker does not yet support this expression.
// Warning: (154-158): Assertion checker does not yet implement type struct C.S memory
// Warning: (154-163): Assertion checker does not yet implement this expression.
// Warning: (154-166): Assertion checker does not yet implement this expression.
// Warning: (182-188): Assertion checker does not yet support this expression.
// Warning: (182-186): Assertion checker does not yet implement type struct C.S memory
// Warning: (182-191): Assertion checker does not yet implement this expression.
// Warning: (182-194): Assertion checker does not yet implement this expression.
// Warning: (209-215): Assertion checker does not yet support this expression.
// Warning: (209-213): Assertion checker does not yet implement type struct C.S memory
// Warning: (209-218): Assertion checker does not yet implement this expression.
// Warning: (202-226): Assertion violation happens here
