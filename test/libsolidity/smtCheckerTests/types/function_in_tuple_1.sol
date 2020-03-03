pragma experimental SMTChecker;

contract K {
	function f() public pure {
		(abi.encode, 2);
	}
}
// ----
// Warning: (76-91): Statement has no effect.
// Warning: (77-80): Assertion checker does not yet implement type abi
