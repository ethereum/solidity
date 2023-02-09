contract C {
    uint x;
    modifier m(uint z) {
		uint y = 3;
        if (z == 10)
            x = 2 + y;
        _;
        if (z == 10)
            x = 4 + y;
    }
    function f() m(10) internal {
        x = 3;
    }
    function g() public {
        x = 0;
        f();
        assert(x == 7);
        // Fails
        assert(x == 6);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (327-341): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
