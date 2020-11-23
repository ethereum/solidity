pragma experimental SMTChecker;
pragma abicoder               v2;

contract C {
	struct T {
		uint y;
		uint[] a;
	}
	struct S {
		uint x;
		T t;
		uint[] a;
		T[] ts;
	}
	function f(S memory s2) public pure {
		S[] memory s1 = new S[](3);
		// Removed because current Spacer seg faults in cex generation.
		//assert(s1.length == 3);
		s1[0].x = 2;
		// Removed because current Spacer seg faults in cex generation.
		//assert(s1[0].x == s2.x);
		s1[1].t.y = 3;
		// Removed because current Spacer seg faults in cex generation.
		//assert(s1[1].t.y == s2.t.y);
		s1[2].a[2] = 4;
		// Removed because current Spacer seg faults in cex generation.
		//assert(s1[2].a[2] == s2.a[2]);
		s1[0].ts[3].y = 5;
		// Removed because current Spacer seg faults in cex generation.
		//assert(s1[0].ts[3].y == s2.ts[3].y);
	}
}
// ----
// Warning 5667: (183-194): Unused function parameter. Remove or comment out the variable name to silence this warning.
