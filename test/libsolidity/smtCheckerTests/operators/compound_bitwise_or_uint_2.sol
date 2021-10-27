contract C {
    struct S {
        uint[] x;
    }
    S s;
	constructor() {
		s.x.push();
		s.x.push();
		s.x.push();
		s.x.push();
	}
    function f(bool b) public {
        if (b)
            s.x[2] |= 1;
		// Removed because of Spacer nondeterminism.
        //assert(s.x[2] != 1);
    }
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n!(s.x.length <= 2)\n
