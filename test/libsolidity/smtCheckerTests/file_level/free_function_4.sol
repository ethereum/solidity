function f()pure {
	ufixed a = uint64(1) + ufixed(2);
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (20-28): Unused local variable.
// Warning 6660: (0-55): Model checker analysis was not possible because file level functions are not supported.
// Warning 6660: (0-55): Model checker analysis was not possible because file level functions are not supported.
