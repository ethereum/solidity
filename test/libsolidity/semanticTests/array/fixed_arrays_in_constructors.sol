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
// gas ir: 262732
// gas irOptimized: 169280
// gas legacy: 192410
// gas legacyOptimized: 151232
// r() -> 4
// ch() -> 3
