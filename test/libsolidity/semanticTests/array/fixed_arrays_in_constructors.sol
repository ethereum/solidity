contract Creator {
    uint256 public r;
    address public ch;

    constructor(address[3] memory s, uint256 x) {
        r = x;
        ch = s[2];
    }
}
// ----
// constructor(): 1, 2, 3, 4 ->
// gas irOptimized: 126363
// gas legacy: 174186
// gas legacyOptimized: 128709
// r() -> 4
// ch() -> 3
