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
// Warning: (143-153): Assertion checker does not yet support the type of this variable.
// Warning: (171-174): Assertion checker does not yet support this expression.
// Warning: (171-172): Assertion checker does not yet implement type struct C.S memory
// Warning: (171-183): Assertion checker does not yet implement such assignments.
// Warning: (194-197): Assertion checker does not yet support this expression.
// Warning: (194-195): Assertion checker does not yet implement type struct C.S memory
// Warning: (187-208): Assertion violation happens here
