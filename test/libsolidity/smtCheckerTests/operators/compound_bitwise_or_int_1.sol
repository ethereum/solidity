pragma experimental SMTChecker;
contract C {
    int[1][20] c;
    function f(bool b) public {
	    require(c[10][0] == 0);
        if (b)
            c[10][0] |= 1;
        assert(c[10][0] == 0 || c[10][0] == 1);
    }
}
// ----
// Warning 6368: (108-113): CHC: Out of bounds access might happen here.
// Warning 6368: (108-116): CHC: Out of bounds access might happen here.
// Warning 6368: (151-156): CHC: Out of bounds access might happen here.
// Warning 6368: (151-159): CHC: Out of bounds access might happen here.
// Warning 6368: (181-186): CHC: Out of bounds access might happen here.
// Warning 6368: (181-189): CHC: Out of bounds access might happen here.
// Warning 6368: (198-203): CHC: Out of bounds access might happen here.
// Warning 6368: (198-206): CHC: Out of bounds access might happen here.
// Warning 6328: (174-212): CHC: Assertion violation might happen here.
