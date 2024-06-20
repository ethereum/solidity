contract C {
    int[2] a;

    constructor() {
        a[0] = 1;
        a[1] = 2;
    }

    function i() public view {
            assert(a[1] > a[0]);
    }
}
// ====
// SMTEngine: chc
// SMTIgnoreInv: no
// SMTSolvers: eld
// SMTTargets: assert
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Info 1180: Contract invariant(s) for :C:\n((a.length = 2) && ((a[1] - a[0]) >= 1))\n
