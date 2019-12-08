pragma experimental SMTChecker;

contract A {
	uint x;
	constructor (uint y) public { assert(x == 0); x = y; }
}

contract B is A {
	constructor () A(2) public { assert(x == 2); }
}

contract C is B {
	function f() public view {
		assert(x == 2);
	}
}
