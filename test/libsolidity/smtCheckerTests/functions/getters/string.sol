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
// Warning 6328: (178-224): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
