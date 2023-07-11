contract C {
    function f(uint x) public pure {
        uint a = 3;
        if (x > 10) {
            a++;
        }
        assert(a == 3);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (127-141): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
