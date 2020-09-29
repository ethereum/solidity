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
// Warning 7812: (157-172): BMC: Assertion violation might happen here.
