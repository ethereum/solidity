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
// Warning 2018: (73-192): Function state mutability can be restricted to pure
// Warning 6328: (172-188): Assertion violation happens here
// Warning 8115: (103-113): Assertion checker does not yet support the type of this variable.
// Warning 7650: (117-120): Assertion checker does not yet support this expression.
// Warning 8364: (117-118): Assertion checker does not yet implement type struct C.S memory
// Warning 8182: (117-124): Assertion checker does not yet implement such assignments.
// Warning 8364: (145-146): Assertion checker does not yet implement type struct C.S memory
// Warning 7650: (165-168): Assertion checker does not yet support this expression.
// Warning 8364: (165-166): Assertion checker does not yet implement type struct C.S memory
// Warning 2683: (158-168): Assertion checker does not yet implement "delete" for this expression.
// Warning 7650: (179-182): Assertion checker does not yet support this expression.
// Warning 8364: (179-180): Assertion checker does not yet implement type struct C.S memory
