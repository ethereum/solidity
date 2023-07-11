contract LoopFor2 {
	uint[] b;
	uint[] c;
	function p() public {
		b.push();
		c.push();
	}
	function testUnboundedForLoop(uint n) public {
		require(n < b.length);
		require(n < c.length);
		require(n > 0 && n < 100);
		b[0] = 900;
		uint[] storage a = b;
		uint i;
		while (i < n) {
			b[i] = i + 1;
			//c[i] = b[i]; // Removed because of Spacer's nondeterminism
			++i;
		}
		//assert(b[0] == c[0]); // Removed because of Spacer's nondeterminism
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (288-292): CHC: Out of bounds access might happen here.
// Warning 6368: (459-463): CHC: Out of bounds access happens here.
// Warning 6328: (452-471): CHC: Assertion violation happens here.
// Warning 6328: (475-494): CHC: Assertion violation happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
