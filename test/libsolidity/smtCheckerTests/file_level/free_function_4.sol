pragma experimental SMTChecker;
function f()pure {
	ufixed a = uint64(1) + ufixed(2);
}
// ----
// Warning 2072: (52-60): Unused local variable.
// Warning 6660: (32-87): Model checker analysis was not possible because file level functions are not supported.
// Warning 6660: (32-87): Model checker analysis was not possible because file level functions are not supported.
