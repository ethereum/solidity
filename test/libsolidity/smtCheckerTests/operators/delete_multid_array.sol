pragma experimental SMTChecker;

contract C {
	uint[] a;
	uint[][] b;
	function f(uint x, uint y, uint v) public {
		a[x] = v;
		delete a;
		assert(a[y] == 0);
	}
 	function g(uint x, uint y, uint v) public {
		b[x][y] = v;
		delete b;
		assert(b[y][x] == 0);
	}
 	function h(uint x, uint y, uint v) public {
		b[x][y] = v;
		delete b[x];
		// Not necessarily the case.
		assert(b[y][x] == 0);
	}
 	function i(uint x, uint y, uint v) public {
		b[x][y] = v;
		delete b[y];
		assert(b[y][x] == 0);
	}
 	function j(uint x, uint y, uint z, uint v) public {
		b[x][y] = v;
		delete b[z];
		// Not necessarily the case.
		assert(b[y][x] == 0);
	}
	function setA(uint x, uint y) public {
		a[x] = y;
	}
	function setB(uint x, uint y, uint z) public {
		b[x][y] = z;
	}
}
// ====
// SMTSolvers: cvc4
// ----
// Warning 4661: (372-392): Assertion violation happens here
// Warning 4661: (617-637): Assertion violation happens here
