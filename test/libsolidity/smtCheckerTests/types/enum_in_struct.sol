pragma experimental SMTChecker;
pragma experimental ABIEncoderV2;

contract C
{
	enum D { Left, Right }
	struct S { uint x; D d; }
	function f(S memory s) public pure {
		s.d = D.Left;
		assert(s.d == D.Left);
	}
}
// ----
// Warning 6328: (187-208): Assertion violation happens here
// Warning 8115: (143-153): Assertion checker does not yet support the type of this variable.
// Warning 7650: (171-174): Assertion checker does not yet support this expression.
// Warning 8364: (171-172): Assertion checker does not yet implement type struct C.S memory
// Warning 8182: (171-183): Assertion checker does not yet implement such assignments.
// Warning 7650: (194-197): Assertion checker does not yet support this expression.
// Warning 8364: (194-195): Assertion checker does not yet implement type struct C.S memory
