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
		// Removed because current Spacer seg faults in cex generation.
		//assert(b[y][x] == 0);
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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (685-705): CHC: Assertion violation happens here.
