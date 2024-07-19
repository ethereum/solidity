contract C {
    bytes20 x;
    function f(bytes16 b) public view {
        b[uint8(x[2])];
    }
}
// ====
// SMTEngine: chc
// SMTSolvers: eld
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
