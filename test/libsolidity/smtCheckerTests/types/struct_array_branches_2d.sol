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
// Warning: (73-211): Function state mutability can be restricted to pure
// Warning: (103-113): Assertion checker does not yet support the type of this variable.
// Warning: (117-120): Assertion checker does not yet support this expression.
// Warning: (117-118): Assertion checker does not yet implement type struct C.S memory
// Warning: (117-123): Assertion checker does not yet implement this expression.
// Warning: (117-126): Assertion checker does not yet implement this expression.
// Warning: (144-147): Assertion checker does not yet support this expression.
// Warning: (144-145): Assertion checker does not yet implement type struct C.S memory
// Warning: (144-150): Assertion checker does not yet implement this expression.
// Warning: (144-153): Assertion checker does not yet implement this expression.
// Warning: (169-172): Assertion checker does not yet support this expression.
// Warning: (169-170): Assertion checker does not yet implement type struct C.S memory
// Warning: (169-175): Assertion checker does not yet implement this expression.
// Warning: (169-178): Assertion checker does not yet implement this expression.
// Warning: (193-196): Assertion checker does not yet support this expression.
// Warning: (193-194): Assertion checker does not yet implement type struct C.S memory
// Warning: (193-199): Assertion checker does not yet implement this expression.
// Warning: (186-207): Assertion violation happens here
