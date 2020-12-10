pragma experimental SMTChecker;
contract K {}
function f() pure {
	(abi.encode, "");
}
// ----
// Warning 6133: (67-83): Statement has no effect.
// Warning 6660: (46-86): Model checker analysis was not possible because file level functions are not supported.
// Warning 6660: (46-86): Model checker analysis was not possible because file level functions are not supported.
