contract K {}
function f() pure {
	(abi.encode, "");
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (67-83): Statement has no effect.
