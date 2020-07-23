pragma experimental SMTChecker;

contract C {
	uint[] a;
	constructor() {
		a.push();
		a.pop();
	}
}
