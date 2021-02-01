pragma experimental SMTChecker;

contract C {
	string public str1 = 'b';

	function f() public view {
		string memory a1 = this.str1();
		assert(keccak256(bytes(a1)) == keccak256(bytes(str1))); // should hold
		assert(keccak256(bytes(a1)) == keccak256('a')); // should fail
	}
}
// ----
// Warning 1218: (211-257): CHC: Error trying to invoke SMT solver.
// Warning 6328: (211-257): CHC: Assertion violation might happen here.
// Warning 4661: (211-257): BMC: Assertion violation happens here.
