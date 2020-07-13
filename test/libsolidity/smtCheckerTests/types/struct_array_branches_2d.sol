pragma experimental SMTChecker;

contract C
{
	struct S { uint[][] a; }
	function f(bool b) public {
		S memory c;
		c.a[0][0] = 0;
		if (b)
			c.a[0][0] = 1;
		else
			c.a[0][0] = 2;
		assert(c.a[0][0] > 0);
	}
}
// ----
// Warning 2018: (73-211): Function state mutability can be restricted to pure
// Warning 6328: (186-207): Assertion violation happens here
// Warning 8115: (103-113): Assertion checker does not yet support the type of this variable.
// Warning 7650: (117-120): Assertion checker does not yet support this expression.
// Warning 8364: (117-118): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (117-123): Assertion checker does not yet implement this expression.
// Warning 9056: (117-126): Assertion checker does not yet implement this expression.
// Warning 7650: (144-147): Assertion checker does not yet support this expression.
// Warning 8364: (144-145): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (144-150): Assertion checker does not yet implement this expression.
// Warning 9056: (144-153): Assertion checker does not yet implement this expression.
// Warning 7650: (169-172): Assertion checker does not yet support this expression.
// Warning 8364: (169-170): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (169-175): Assertion checker does not yet implement this expression.
// Warning 9056: (169-178): Assertion checker does not yet implement this expression.
// Warning 7650: (193-196): Assertion checker does not yet support this expression.
// Warning 8364: (193-194): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (193-199): Assertion checker does not yet implement this expression.
