contract A {
	uint x;
	function h() public view {
		assert(x == 0);
	}
}

contract B is A {
	function g() public view {
		assert(x == 0);
	}
}

contract C is B {
	function f() public view {
		assert(x == 0);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Info 1180: Contract invariant(s) for :A:\n(x <= 0)\nContract invariant(s) for :C:\n(x <= 0)\nContract invariant(s) for :B:\n(x <= 0)\n
