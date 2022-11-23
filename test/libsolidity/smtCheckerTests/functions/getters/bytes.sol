contract C {
	bytes public str2 = 'c';

	function f() public view {
		bytes memory a2 = this.str2();
		assert(keccak256(a2) == keccak256(str2)); // should hold
		assert(keccak256(a2) == keccak256('a')); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (162-201): CHC: Assertion violation happens here.\nCounterexample:\nstr2 = [0x63]\na2 = [0x63]\n\nTransaction trace:\nC.constructor()\nState: str2 = [0x63]\nC.f()
