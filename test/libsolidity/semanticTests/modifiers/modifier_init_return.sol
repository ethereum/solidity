contract C {
    modifier m(bool condition) {
        if (condition) _;
    }

    function f(uint x) public m(x >= 10) returns (uint[5] memory r) {
        r[2] = 3;
    }
}
// ----
// f(uint256): 9 -> 0x00, 0x00, 0x00, 0x00, 0x00
// f(uint256): 10 -> 0x00, 0x00, 3, 0x00, 0x00
