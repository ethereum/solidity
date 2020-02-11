contract C {
    function f(uint n, uint k) public returns(uint) {
        uint[][] memory rows = new uint[][](n + 1);
        for (uint i = 1; i <= n; i++) {
            rows[i] = new uint[](i);
            rows[i][0] = rows[i][rows[i].length - 1] = 1;
            for (uint j = 1; j < i - 1; j++)
                rows[i][j] = rows[i - 1][j - 1] + rows[i - 1][j];
        }
        return rows[n][k - 1];
    }
}

// ----
// f(uint256,uint256): encodeArgs(3), 1)) -> 1
// f(uint256,uint256):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1]" -> "1"
// f(uint256,uint256): encodeArgs(9), 5)) -> 70
// f(uint256,uint256):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5]" -> "70"
