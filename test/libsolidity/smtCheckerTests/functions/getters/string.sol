contract C {
	string public str1 = 'b';

	function f() public view {
		string memory a1 = this.str1();
		assert(keccak256(bytes(a1)) == keccak256(bytes(str1))); // should hold
		assert(keccak256(bytes(a1)) == keccak256('a')); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 1218: (178-224): CHC: Error trying to invoke SMT solver.
// Warning 6328: (178-224): CHC: Assertion violation might happen here.
// Warning 4661: (178-224): BMC: Assertion violation happens here.
