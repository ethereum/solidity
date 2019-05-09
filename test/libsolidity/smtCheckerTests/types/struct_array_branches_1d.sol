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
// Warning: (71-197): Function state mutability can be restricted to pure
// Warning: (101-111): Assertion checker does not yet support the type of this variable.
// Warning: (115-118): Assertion checker does not yet support this expression.
// Warning: (115-121): Assertion checker does not yet implement this expression.
// Warning: (115-121): Assertion checker does not yet implement this expression.
// Warning: (139-142): Assertion checker does not yet support this expression.
// Warning: (139-145): Assertion checker does not yet implement this expression.
// Warning: (139-145): Assertion checker does not yet implement this expression.
// Warning: (161-164): Assertion checker does not yet support this expression.
// Warning: (161-167): Assertion checker does not yet implement this expression.
// Warning: (161-167): Assertion checker does not yet implement this expression.
// Warning: (182-185): Assertion checker does not yet support this expression.
// Warning: (182-188): Assertion checker does not yet implement this expression.
// Warning: (175-193): Assertion violation happens here
