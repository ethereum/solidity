contract C {
    int[1][20] c;
    function f(bool b) public {
	    require(c[10][0] == 0);
        if (b)
            c[10][0] |= 1;
        assert(c[10][0] == 0 || c[10][0] == 1);
    }
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6368: (76-81): CHC: Out of bounds access might happen here.
// Warning 6368: (119-124): CHC: Out of bounds access might happen here.
// Warning 6368: (119-127): CHC: Out of bounds access might happen here.
// Warning 6368: (149-154): CHC: Out of bounds access might happen here.
// Warning 6368: (149-157): CHC: Out of bounds access might happen here.
// Warning 6368: (166-171): CHC: Out of bounds access might happen here.
// Warning 6368: (166-174): CHC: Out of bounds access might happen here.
// Warning 6328: (142-180): CHC: Assertion violation might happen here.
