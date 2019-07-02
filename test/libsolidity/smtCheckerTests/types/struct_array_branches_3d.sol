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
// Warning: (110-120): Assertion checker does not yet support the type of this variable.
// Warning: (124-127): Assertion checker does not yet support this expression.
// Warning: (124-125): Assertion checker does not yet implement type struct C.S memory
// Warning: (124-130): Assertion checker does not yet implement this expression.
// Warning: (124-136): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (154-157): Assertion checker does not yet support this expression.
// Warning: (154-155): Assertion checker does not yet implement type struct C.S memory
// Warning: (154-160): Assertion checker does not yet implement this expression.
// Warning: (154-166): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (182-185): Assertion checker does not yet support this expression.
// Warning: (182-183): Assertion checker does not yet implement type struct C.S memory
// Warning: (182-188): Assertion checker does not yet implement this expression.
// Warning: (182-194): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (209-212): Assertion checker does not yet support this expression.
// Warning: (209-210): Assertion checker does not yet implement type struct C.S memory
// Warning: (209-215): Assertion checker does not yet implement this expression.
// Warning: (202-226): Assertion violation happens here
