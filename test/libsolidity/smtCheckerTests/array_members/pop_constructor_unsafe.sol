pragma experimental SMTChecker;

contract C {
	uint[] a;
	constructor() {
		a.pop();
	}
}
// ----
// Warning 2529: (76-83): Empty array "pop" detected here
