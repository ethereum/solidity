contract C {
	uint x;
	function f(uint _x) public {
		x = _x;
	}
	constructor(address _a) {
		_a.call("aaaa");
		assert(x == 0); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 9302: (94-109): Return value of low-level calls not used.
