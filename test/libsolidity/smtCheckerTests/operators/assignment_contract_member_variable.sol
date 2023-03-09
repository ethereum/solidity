contract A {
	int x;
	int y;
	function a() public {
		require(A.x < 100);
		A.y = A.x++;
		assert(A.y == A.x - 1);
		// Fails
		// assert(A.y == 0); // Disabled because of nondeterminism in Spacer
		A.y = ++A.x;
		assert(A.y == A.x);
		delete A.x;
		assert(A.x == 0);
		A.y = A.x--;
		assert(A.y == A.x + 1);
		assert(A.y == 0);
		A.y = --A.x;
		assert(A.y == A.x);
		A.x += 10;
		// Fails
		assert(A.y == 0);
		assert(A.y + 10 == A.x);
		A.x -= 10;
		assert(A.y == A.x);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTIgnoreInv: yes
// ----
// Warning 6328: (392-408): CHC: Assertion violation happens here.
// Info 1391: CHC: 17 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
