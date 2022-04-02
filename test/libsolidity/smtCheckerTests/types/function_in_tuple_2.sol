contract K {
	function f() public pure {
		(abi.encode, "");
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (43-59='(abi.encode, "")'): Statement has no effect.
