contract C {
    uint x;

    function inc() internal returns (uint) {
        // BMC inlines function calls and also looks at functions in isolation,
        // therefore it says once that this is safe, when called by `f`,
        // but also that it is unsafe just looking at this function (false positive).
        ++x;
        return x;
    }

    function f() public {
        require(x < 1000);
        uint z = x;
        uint y = (inc() < 3) ? inc() : inc();

        assert(y == x);
        assert(x == z + 2); // should hold
        assert(x != z + 2); // should fail
    }
}
// ====
// SMTEngine: bmc
// ----
// Warning 2661: (318-321): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4661: (543-561): BMC: Assertion violation happens here.
// Info 6002: BMC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
