contract A {
	constructor() payable {}
}

contract B {
	function f() public payable {
		require(address(this).balance == 100);
		A a = new A{value: 50}();
		assert(address(this).balance == 50); // should hold
		assert(address(this).balance == 60); // should fail
		assert(address(a).balance >= 50); // should hold
		assert(address(a).balance == 50); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// ----
// Warning 6328: (211-246): CHC: Assertion violation happens here.
// Warning 6328: (316-348): CHC: Assertion violation happens here.
