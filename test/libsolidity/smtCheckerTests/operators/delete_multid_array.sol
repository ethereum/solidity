contract C {
	uint[] a;
	uint[][] b;
	function p() public { a.push(); }
	function q() public { b.push().push(); }
	function f(uint x, uint v) public {
		require(x < a.length);
		a[x] = v;
		delete a;
		assert(a.length == 0);
	}
 	function g(uint x, uint y, uint v) public {
		require(x < b.length);
		require(y < b[x].length);
		b[x][y] = v;
		delete b;
		assert(b.length == 0);
	}
 	function h(uint x, uint y, uint v) public {
		require(x < b.length);
		require(y < b[x].length);
		b[x][y] = v;
		delete b[x];
		assert(b[x].length == 0);
	}
 	function i(uint x, uint y, uint v) public {
		require(x < b.length);
		require(y < b[x].length);
		b[x][y] = v;
		require(y < b.length);
		delete b[y];
		assert(b[y].length == 0);
	}
 	function j(uint x, uint y, uint z, uint v) public {
		require(x < b.length);
		require(y < b[x].length);
		b[x][y] = v;
		require(z < b.length);
		delete b[z];
		// Not necessarily the case.
		require(y < b.length);
		require(x < b[x].length);
		// Disabled because of Spacer nondeterminism.
		//assert(b[y][x] == 0);
	}
	function setA(uint x, uint y) public {
		require(x < a.length);
		a[x] = y;
	}
	function setB(uint x, uint y, uint z) public {
		require(x < b.length);
		require(y < b[x].length);
		b[x][y] = z;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Info 1391: CHC: 27 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
