contract C {
	function f() public payable {
		assert((msg).value == 10);
		assert((true ? msg : msg).value == 12);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// SMTIgnoreCex: yes
// ----
// Warning 6328: (46-71): CHC: Assertion violation happens here.
// Warning 6328: (75-113): CHC: Assertion violation happens here.
