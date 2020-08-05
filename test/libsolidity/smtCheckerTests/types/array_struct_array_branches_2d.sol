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
// Warning 6328: (202-226): Assertion violation happens here
// Warning 7650: (124-130): Assertion checker does not yet support this expression.
// Warning 8364: (124-128): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (124-133): Assertion checker does not yet implement this expression.
// Warning 9056: (124-136): Assertion checker does not yet implement this expression.
// Warning 7650: (154-160): Assertion checker does not yet support this expression.
// Warning 8364: (154-158): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (154-163): Assertion checker does not yet implement this expression.
// Warning 9056: (154-166): Assertion checker does not yet implement this expression.
// Warning 7650: (182-188): Assertion checker does not yet support this expression.
// Warning 8364: (182-186): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (182-191): Assertion checker does not yet implement this expression.
// Warning 9056: (182-194): Assertion checker does not yet implement this expression.
// Warning 7650: (209-215): Assertion checker does not yet support this expression.
// Warning 8364: (209-213): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (209-218): Assertion checker does not yet implement this expression.
