contract Test {
    function set(uint24[3][4] memory x) public {
        x[2][2] = 1;
        x[3][2] = 7;
    }

    function f() public returns (uint24[3][4] memory) {
        uint24[3][4] memory data;
        set(data);
        return data;
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x07
