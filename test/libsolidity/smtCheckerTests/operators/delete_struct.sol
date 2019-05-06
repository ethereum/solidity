pragma experimental SMTChecker;

contract C
{
	struct S
	{
		uint x;
	}
	function f(bool b) public {
		S memory s;
		s.x = 2;
		if (b)
			delete s;
		else
			delete s.x;
		assert(s.x == 0);
	}
}
// ----
// Warning: (73-192): Function state mutability can be restricted to pure
// Warning: (103-113): Assertion checker does not yet support the type of this variable.
// Warning: (117-120): Assertion checker does not yet support this expression.
// Warning: (117-124): Assertion checker does not yet implement such assignments.
// Warning: (165-168): Assertion checker does not yet support this expression.
// Warning: (158-168): Assertion checker does not yet implement "delete" for this expression.
// Warning: (179-182): Assertion checker does not yet support this expression.
// Warning: (172-188): Assertion violation happens here
