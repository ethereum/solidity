pragma experimental SMTChecker;

contract C
{
	struct S { uint[][][] a; }
	function f(bool b) public pure {
		S memory c;
		c.a[0][0][0] = 0;
		if (b)
			c.a[0][0][0] = 1;
		else
			c.a[0][0][0] = 2;
		assert(c.a[0][0][0] > 0);
	}
}
// ----
// Warning 6328: (202-226): Assertion violation happens here
// Warning 8115: (110-120): Assertion checker does not yet support the type of this variable.
// Warning 7650: (124-127): Assertion checker does not yet support this expression.
// Warning 8364: (124-125): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (124-130): Assertion checker does not yet implement this expression.
// Warning 9056: (124-136): Assertion checker does not yet implement this expression.
// Warning 7650: (154-157): Assertion checker does not yet support this expression.
// Warning 8364: (154-155): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (154-160): Assertion checker does not yet implement this expression.
// Warning 9056: (154-166): Assertion checker does not yet implement this expression.
// Warning 7650: (182-185): Assertion checker does not yet support this expression.
// Warning 8364: (182-183): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (182-188): Assertion checker does not yet implement this expression.
// Warning 9056: (182-194): Assertion checker does not yet implement this expression.
// Warning 7650: (209-212): Assertion checker does not yet support this expression.
// Warning 8364: (209-210): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (209-215): Assertion checker does not yet implement this expression.
