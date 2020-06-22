pragma experimental SMTChecker;

contract C
{
	enum D { Left, Right }
	D d;
	function f(D _d) public {
		d = _d;
		assert(d != _d);
	}
}
// ----
// Warning 4661: (115-130): Assertion violation happens here
