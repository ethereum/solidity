pragma experimental SMTChecker;

contract C {
	uint[] a;
	function f(uint l) public {
		for (uint i = 0; i < l; ++i) {
			a.push();
			a.pop();
		}
	}
}
// ----
// Warning 4984: (112-115): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
