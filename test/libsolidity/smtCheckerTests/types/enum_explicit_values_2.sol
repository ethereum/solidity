pragma experimental SMTChecker;

contract C
{
	enum D { Left, Right }
	D d;
	function f(D _a) public {
		require(_a == D.Left);
		d = D.Left;
		assert(d != _a);
	}
}
// ----
// Warning: (144-159): Assertion violation happens here
