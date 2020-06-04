pragma experimental SMTChecker;

contract C {
	uint[] a;
	constructor() public {
		a.push();
		a.pop();
	}
}
