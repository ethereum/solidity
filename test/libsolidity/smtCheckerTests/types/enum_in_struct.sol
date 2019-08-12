pragma experimental SMTChecker;

contract C
{
	enum D { Left, Right }
	struct S { uint x; D d; }
	function f(S memory s) internal pure {
		s.d = D.Left;
		assert(s.d == D.Left);
	}
}
// ----
// Warning: (109-119): Assertion checker does not yet support the type of this variable.
// Warning: (139-142): Assertion checker does not yet support this expression.
// Warning: (139-140): Assertion checker does not yet implement type struct C.S memory
// Warning: (139-151): Assertion checker does not yet implement such assignments.
// Warning: (162-165): Assertion checker does not yet support this expression.
// Warning: (162-163): Assertion checker does not yet implement type struct C.S memory
// Warning: (155-176): Assertion violation happens here
