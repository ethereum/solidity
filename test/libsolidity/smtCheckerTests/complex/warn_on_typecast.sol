contract C {
    function f() public pure returns (uint) {
        return uint8(1);
    }
}
// ====
// SMTEngine: all
// ----
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
