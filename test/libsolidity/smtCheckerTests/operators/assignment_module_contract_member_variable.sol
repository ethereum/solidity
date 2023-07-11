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
// Warning 6328: (AASource:159-178): CHC: Assertion violation happens here.
// Warning 6328: (AASource:370-386): CHC: Assertion violation happens here.
// Info 1391: CHC: 16 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
