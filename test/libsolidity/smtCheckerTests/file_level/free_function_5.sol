contract K {}
function f() pure {
	(abi.encode, "");
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (35-51='(abi.encode, "")'): Statement has no effect.
