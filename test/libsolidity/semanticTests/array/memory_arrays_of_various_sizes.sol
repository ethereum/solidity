// Computes binomial coefficients the chinese way
contract C {
    function f(uint256 n, uint256 k) public returns (uint256) {
        uint256[][] memory rows = new uint256[][](n + 1);
        for (uint256 i = 1; i <= n; i++) {
            rows[i] = new uint256[](i);
            rows[i][0] = rows[i][rows[i].length - 1] = 1;
            for (uint256 j = 1; j < i - 1; j++)
                rows[i][j] = rows[i - 1][j - 1] + rows[i - 1][j];
        }
        return rows[n][k - 1];
    }
}
// ====
// compileViaYul: also
// ----
// f(uint256,uint256): 3, 1 -> 1
// f(uint256,uint256): 9, 5 -> 70
