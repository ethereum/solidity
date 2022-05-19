contract C {
    function f(bytes calldata x) public returns (uint r) {
        assembly { r := x.offset }
    }

    function f(uint, bytes calldata x, uint) public returns (uint r, uint v) {
        assembly {
            r := x.offset
            v := x.length
        }
    }
}
// ====
// compileToEwasm: also
// ----
// f(bytes): 0x20, 0, 0 -> 0x44
// f(bytes): 0x22, 0, 0, 0 -> 0x46
// f(uint256,bytes,uint256): 7, 0x60, 8, 2, 0 -> 0x84, 2
// f(uint256,bytes,uint256): 0, 0, 0 -> 0x24, 0x00
