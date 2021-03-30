pragma experimental SMTChecker;

contract C
{
	function f(address a) public view {
		assert(a == address(this));
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (85-111): CHC: Assertion violation happens here.
