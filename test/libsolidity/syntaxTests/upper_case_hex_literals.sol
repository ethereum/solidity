contract test {

    function f() public pure returns (uint256) {
        uint256 a = 0x1234aAbcC;
        uint256 b = 0x1234ABCDEF;
        return a + b;
    }
}
// ----
