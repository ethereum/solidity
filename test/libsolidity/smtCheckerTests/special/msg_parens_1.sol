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
// TypeError 9717: (90-93): Invalid mobile type in true expression.
// TypeError 3703: (96-99): Invalid mobile type in false expression.
