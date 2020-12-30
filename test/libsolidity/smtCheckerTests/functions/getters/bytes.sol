pragma experimental SMTChecker;

contract C {
	bytes public str2 = 'c';

	function f() public view {
		bytes memory a2 = this.str2();
		assert(keccak256(a2) == keccak256(str2)); // should hold
		assert(keccak256(a2) == keccak256('a')); // should fail
	}
}
// ----
// Warning 1218: (195-234): CHC: Error trying to invoke SMT solver.
// Warning 6328: (195-234): CHC: Assertion violation might happen here.
// Warning 4661: (195-234): BMC: Assertion violation happens here.
