contract A {
	uint x;
	constructor (uint y) { assert(x == 0); x = y; }
}

contract B is A {
	constructor () A(2) { assert(x == 2); }
}

contract C is B {
	function f() public view {
		assert(x == 2);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreInv: yes
// SMTSolvers: z3
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
