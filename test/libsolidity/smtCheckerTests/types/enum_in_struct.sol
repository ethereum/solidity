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
