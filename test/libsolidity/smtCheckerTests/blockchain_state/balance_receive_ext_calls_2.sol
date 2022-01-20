contract C {
	function f(address _a) public {
		uint x = address(this).balance;
		_a.call("");
		assert(address(this).balance == x); // should fail
		assert(address(this).balance >= x); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTIgnoreInv: yes
// ----
// Warning 9302: (82-93): Return value of low-level calls not used.
// Warning 6328: (97-131): CHC: Assertion violation happens here.
