pragma experimental SMTChecker;

contract C
{
	struct S { uint[] a; }
	function f(bool b) public {
		S memory c;
		c.a[0] = 0;
		if (b)
			c.a[0] = 1;
		else
			c.a[0] = 2;
		assert(c.a[0] > 0);
	}
}
// ----
// Warning 2018: (71-197): Function state mutability can be restricted to pure
// Warning 6328: (175-193): Assertion violation happens here
// Warning 8115: (101-111): Assertion checker does not yet support the type of this variable.
// Warning 7650: (115-118): Assertion checker does not yet support this expression.
// Warning 8364: (115-116): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (115-121): Assertion checker does not yet implement this expression.
// Warning 9056: (115-121): Assertion checker does not yet implement this expression.
// Warning 7650: (139-142): Assertion checker does not yet support this expression.
// Warning 8364: (139-140): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (139-145): Assertion checker does not yet implement this expression.
// Warning 9056: (139-145): Assertion checker does not yet implement this expression.
// Warning 7650: (161-164): Assertion checker does not yet support this expression.
// Warning 8364: (161-162): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (161-167): Assertion checker does not yet implement this expression.
// Warning 9056: (161-167): Assertion checker does not yet implement this expression.
// Warning 7650: (182-185): Assertion checker does not yet support this expression.
// Warning 8364: (182-183): Assertion checker does not yet implement type struct C.S memory
// Warning 9118: (182-188): Assertion checker does not yet implement this expression.
