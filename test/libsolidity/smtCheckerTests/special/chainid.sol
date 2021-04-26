contract C {
    function f() public view returns (uint) {
        return block.chainid + 0; // Overflow not possible!
    }
}
// ====
// SMTEngine: all
// ----
