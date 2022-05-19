contract C {
    function f(uint[2][] calldata x) public returns (uint o, uint l, uint s) {
        assembly { l := x.length o := x.offset }
        uint[2] calldata t = x[1];
        // statically-sized arrays only use one slot, so we read directly.
        assembly { s := t }
    }
}
// ====
// compileToEwasm: also
// ----
// f(uint256[2][]): 0x20, 2, 1, 2, 3, 4 -> 0x44, 2, 0x84
