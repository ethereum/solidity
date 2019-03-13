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
// Warning: (113-121): Type conversion is not yet fully supported and might yield false positives.
