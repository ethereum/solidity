pragma experimental SMTChecker;

contract C
{
	enum D { Left, Right }
	function f(D _a) public pure {
		uint x = uint(_a);
		assert(x < 10);
	}
}
// ----
