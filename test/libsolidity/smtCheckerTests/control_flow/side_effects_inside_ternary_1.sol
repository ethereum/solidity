contract C {
    function f() public pure {
        uint x;
        uint y = (++x < 3) ? ++x : --x;

        assert(y == x); // should hold;
        assert(x == 2); // should hold
        assert(x != 2); // should fail
    }
}
// ====
// SMTEngine: bmc
// ----
// Warning 6838: (77-86): BMC: Condition is always true.
// Warning 4661: (188-202): BMC: Assertion violation happens here.
// Info 6002: BMC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
