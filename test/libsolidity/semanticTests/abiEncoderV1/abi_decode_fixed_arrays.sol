contract C {
    function f(uint16[3] memory a, uint16[2][3] memory b, uint i, uint j, uint k)
            public pure returns (uint, uint) {
        return (a[i], b[j][k]);
    }
}
// ----
// f(uint16[3],uint16[2][3],uint256,uint256,uint256): 1, 2, 3, 11, 12, 21, 22, 31, 32, 1, 2, 1 -> 2, 32
