pragma experimental SMTChecker;

contract K {
	function f() public pure {
		(abi.encode, "");
	}
}
// ----
// Warning: (76-92): Statement has no effect.
// Warning: (77-80): Assertion checker does not yet implement type abi
