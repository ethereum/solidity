contract C {
    struct S { int x; }
    S s = S(1);

    function i() public view {
            assert(s.x > 0);
    }
}
// ====
// SMTEngine: chc
// SMTIgnoreInv: no
// SMTSolvers: eld
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Info 1180: Contract invariant(s) for :C:\n(s.x = 1)\n
