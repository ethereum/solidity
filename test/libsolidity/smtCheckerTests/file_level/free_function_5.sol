contract K {}
function f() pure {
	(abi.encode, "");
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (35-51): Statement has no effect.
