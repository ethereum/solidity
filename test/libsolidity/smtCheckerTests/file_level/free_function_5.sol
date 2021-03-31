contract K {}
function f() pure {
	(abi.encode, "");
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (35-51): Statement has no effect.
// Warning 6660: (14-54): Model checker analysis was not possible because file level functions are not supported.
// Warning 6660: (14-54): Model checker analysis was not possible because file level functions are not supported.
