contract C {
    uint x;

    function inc() internal returns (uint) {
        // BMC inlines function calls and also looks at functions in isolation,
        // therefore it says once that this is safe, when called by `f`,
        // but also that it is unsafe just looking at this function (false positive).
        ++x;
        return x;
    }

    function inc2() internal returns (uint) {
        return inc();
    }

    function f() public {
        require(x < 1000);
        uint y = x;
        if (inc2() < 3) {}

        assert(x == y + 1); // should hold
        assert(x != y + 1); // should fail
    }
}
// ====
// SMTEngine: bmc
// ----
// Warning 2661: (318-321): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4661: (575-593): BMC: Assertion violation happens here.
// Info 6002: BMC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
