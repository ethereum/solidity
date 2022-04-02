contract C {
    uint[1] c;
    function f(bool b) public {
        require(c[0] == 0);
        if (b)
            c[0] |= 1;
        assert(c[0] <= 1);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (76-80='c[0]'): CHC: Out of bounds access might happen here.
// Warning 6368: (115-119='c[0]'): CHC: Out of bounds access might happen here.
// Warning 6368: (141-145='c[0]'): CHC: Out of bounds access might happen here.
// Warning 6328: (134-151='assert(c[0] <= 1)'): CHC: Assertion violation might happen here.
