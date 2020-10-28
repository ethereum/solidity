pragma experimental SMTChecker;
contract C {
    struct S {
        uint[] x;
    }
    S s;
    function f(bool b) public {
        if (b)
            s.x[2] |= 1;
		// Removed because of Spacer nondeterminism.
        //assert(s.x[2] != 1);
    }
}
