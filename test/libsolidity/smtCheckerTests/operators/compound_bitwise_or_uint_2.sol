pragma experimental SMTChecker;
contract C {
    struct S {
        uint[] x;
    }
    S s;
    function f(bool b) public {
        if (b)
            s.x[2] |= 1;
        assert(s.x[2] != 1);
    }
}
// ----
// Warning 7812: (173-192): BMC: Assertion violation might happen here.
