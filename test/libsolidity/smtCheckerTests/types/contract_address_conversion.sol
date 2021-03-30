pragma experimental SMTChecker;

contract C
{
	function f(C c, address a) public pure {
		assert(address(c) == a);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (90-113): CHC: Assertion violation happens here.
