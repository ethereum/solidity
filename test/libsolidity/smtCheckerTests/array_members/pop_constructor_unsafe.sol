pragma experimental SMTChecker;

contract C {
	uint[] a;
	constructor() public {
		a.pop();
	}
}
// ----
// Warning: (83-90): Empty array "pop" detected here.
