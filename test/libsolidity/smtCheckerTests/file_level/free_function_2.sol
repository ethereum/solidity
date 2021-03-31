contract C {
	function g() external {
		f();
	}
}
function f() {}
// ====
// SMTEngine: all
// ----
// Warning 6660: (50-65): Model checker analysis was not possible because file level functions are not supported.
// Warning 6660: (50-65): Model checker analysis was not possible because file level functions are not supported.
