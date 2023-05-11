contract C {
    function f() public pure {
        uint x;
        if (++x < 3) {}

        assert(x == 1); // should hold
        assert(x != 1); // should fail
    }
}
// ====
// SMTEngine: bmc
// ----
// Warning 6838: (72-79): BMC: Condition is always true.
// Warning 4661: (132-146): BMC: Assertion violation happens here.
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
