contract C {
    function f() public view returns (uint) {
        return block.chainid + 0; // Overflow not possible!
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
