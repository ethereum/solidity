contract Test {
    function del() public returns (uint24[3][4] memory) {
        uint24[3][4] memory x;
        for (uint24 i = 0; i < x.length; i ++)
            for (uint24 j = 0; j < x[i].length; j ++)
                x[i][j] = i * 0x10 + j;
        delete x[1];
        delete x[3][2];
        return x;
    }
}
// ====
// compileViaYul: also
// ----
// del() -> 0, 1, 2, 0, 0, 0, 0x20, 0x21, 0x22, 0x30, 0x31, 0
