contract Test {
    uint24[3][][4] data;

    function set(uint24[3][][4] memory x)
        internal
        returns (uint24[3][][4] memory)
    {
        x[1][2][2] = 1;
        x[1][3][2] = 7;
        return x;
    }

    function f() public returns (uint24[3][] memory) {
        while (data[1].length < 4) data[1].push();
        return set(data)[1];
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x20, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x07
