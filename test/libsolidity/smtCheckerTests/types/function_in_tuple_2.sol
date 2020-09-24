pragma experimental SMTChecker;

contract K {
	function f() public pure {
		(abi.encode, "");
	}
}
// ----
// Warning 6133: (76-92): Statement has no effect.
