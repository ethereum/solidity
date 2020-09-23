pragma experimental SMTChecker;
contract C {
    struct S {
        uint x;
    }
    S s;
    function f(bool b) public {
        s.x |= b ? 1 : 2;
        assert(s.x > 0);
    }
}
// ----
// Warning 1218: (157-172): Error trying to invoke SMT solver.
// Warning 7812: (157-172): Assertion violation might happen here.
