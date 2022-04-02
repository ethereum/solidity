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
// ====
// SMTEngine: all
// ----
// Warning 6328: (125-140='assert(s.x > 0)'): CHC: Assertion violation might happen here.
// Warning 7812: (125-140='assert(s.x > 0)'): BMC: Assertion violation might happen here.
