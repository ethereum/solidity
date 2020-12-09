pragma experimental SMTChecker;
contract C {
	function g() external {
		f();
	}
}
function f() {}
// ----
// Warning 6660: (82-97): Model checker analysis was not possible because file level functions are not supported.
// Warning 6660: (82-97): Model checker analysis was not possible because file level functions are not supported.
