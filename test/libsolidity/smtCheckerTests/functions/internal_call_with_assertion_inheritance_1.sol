contract A {
	uint x;
	function f() internal {
		assert(x == 1);
		--x;
	}
}

contract C is A {
	constructor() {
		assert(x == 0);
		++x;
		f();
		assert(x == 0);
	}
}
// ====
// SMTEngine: all
// ----
