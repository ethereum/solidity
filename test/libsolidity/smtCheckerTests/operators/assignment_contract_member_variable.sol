pragma experimental SMTChecker;
contract A {
	int x;
	int y;
	function a() public {
		require(A.x < 100);
		A.y = A.x++;
		assert(A.y == A.x - 1);
		// Fails
		assert(A.y == 0);
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
// ----
// Warning 6328: (160-176): CHC: Assertion violation happens here.
// Warning 6328: (373-389): CHC: Assertion violation happens here.
