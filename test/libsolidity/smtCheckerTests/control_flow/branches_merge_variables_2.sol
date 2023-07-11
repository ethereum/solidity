// Positive branch touches variable a, but assertion should still hold.
contract C {
    function f(uint x) public pure {
        uint a = 3;
        if (x > 10) {
            a = 3;
        }
        assert(a == 3);
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
