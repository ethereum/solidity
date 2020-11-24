pragma abicoder               v2;
contract C {
    function f(uint256[][] calldata x) external pure {
        x[0][1:2];
        uint256 a = x[0][1:2][1:2][1:][3:][0];
        uint256 b = x[1][1:][3:4][1:][2:3][0];
        a; b;
    }
}
// ----
