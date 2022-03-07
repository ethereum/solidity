contract Creator {
    uint256 public r;
    address public ch;

    constructor(address[3] memory s, uint256 x) {
        r = x;
        ch = s[2];
    }
}
// ====
// compileViaYul: also
// ----
// constructor(): 1, 2, 3, 4 ->
// gas irOptimized: 128995
// gas legacy: 176789
// gas legacyOptimized: 129585
// r() -> 4
// ch() -> 3
