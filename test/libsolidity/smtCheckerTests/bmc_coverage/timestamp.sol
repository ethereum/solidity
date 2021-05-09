contract C {
    function f() public view returns (uint) {
        uint b = block.timestamp;
        uint a = b + 0; // Overflow not possible!
        return a;
    }
}
// ====
// SMTEngine: bmc
// ----
