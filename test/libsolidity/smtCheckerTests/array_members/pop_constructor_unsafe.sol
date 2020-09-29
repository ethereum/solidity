pragma experimental SMTChecker;

contract C {
	uint[] a;
	constructor() {
		a.pop();
	}
}
// ----
// Warning 2529: (76-83): CHC: Empty array "pop" detected here.
