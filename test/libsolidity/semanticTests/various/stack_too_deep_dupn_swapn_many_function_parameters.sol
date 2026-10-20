// The experimental SSA CFG code transform still assumes a reachable stack depth of 16 and does
// not support the extended depth provided by DUPN/SWAPN (EIP-8024) yet, so it is disabled here.
contract C {
    function f(
        uint256 a1,
        uint256 a2,
        uint256 a3,
        uint256 a4,
        uint256 a5,
        uint256 a6,
        uint256 a7,
        uint256 a8,
        uint256 a9,
        uint256 a10,
        uint256 a11,
        uint256 a12,
        uint256 a13,
        uint256 a14,
        uint256 a15,
        uint256 a16,
        uint256 a17,
        uint256 a18,
        uint256 a19,
        uint256 a20
    ) public pure returns (uint256) {
        return a1 * 1 + a2 * 2 + a3 * 3 + a4 * 4 + a5 * 5 + a6 * 6 + a7 * 7 + a8 * 8 + a9 * 9 + a10 * 10 + a11 * 11 + a12 * 12 + a13 * 13 + a14 * 14 + a15 * 15 + a16 * 16 + a17 * 17 + a18 * 18 + a19 * 19 + a20 * 20;
    }
}
// ====
// EVMVersion: >=amsterdam
// compileViaSSACFG: false
// ----
// f(uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256,uint256): 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 -> 2870
