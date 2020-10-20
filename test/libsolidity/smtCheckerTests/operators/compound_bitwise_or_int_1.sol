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
// Warning 6328: (174-212): CHC: Assertion violation might happen here.
