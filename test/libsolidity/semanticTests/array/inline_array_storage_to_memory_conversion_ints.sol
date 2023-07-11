contract C {
    function f() public returns (uint256 x, uint256 y) {
        x = 3;
        y = 6;
        uint256[2] memory z = [x, y];
        return (z[0], z[1]);
    }
}
// ----
// f() -> 3, 6
