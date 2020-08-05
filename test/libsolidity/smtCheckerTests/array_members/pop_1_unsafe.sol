pragma experimental SMTChecker;

contract C {
	uint[] a;
	function f() public {
		a.pop();
	}
}
// ----
// Warning 2529: (82-89): Empty array "pop" detected here
