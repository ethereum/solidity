==== Source: AASource ====
import "AASource" as AA;
contract A {
	int x;
	int y;
	function a() public {
		require(A.x < 100);
		AA.A.y = A.x++;
		assert(A.y == AA.A.x - 1);
		// Fails
		assert(AA.A.y == 0);
		A.y = ++AA.A.x;
		assert(A.y == A.x);
		delete AA.A.x;
		assert(A.x == 0);
		A.y = A.x--;
		assert(AA.A.y == AA.A.x + 1);
		A.y = --A.x;
		assert(A.y == A.x);
		AA.A.x += 10;
		// Fails
		assert(A.y == 0);
		assert(A.y + 10 == A.x);
		A.x -= 10;
		assert(AA.A.y == A.x);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (AASource:159-178): CHC: Assertion violation happens here.\nCounterexample:\nx = (- 1), y = (- 2)\n\nTransaction trace:\nA.constructor()\nState: x = 0, y = 0\nA.a()\nState: x = (- 2), y = (- 2)\nA.a()
// Warning 6328: (AASource:370-386): CHC: Assertion violation happens here.\nCounterexample:\nx = 8, y = (- 2)\n\nTransaction trace:\nA.constructor()\nState: x = 0, y = 0\nA.a()
// Info 1180: Contract invariant(s) for AASource:A:\n(((x = 0) && (y = 0)) || ((x = (- 2)) && (y = (- 2))))\n
