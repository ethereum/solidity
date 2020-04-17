contract Creator {
    uint256 public r;
    address public ch;

    constructor(address[3] memory s, uint256 x) public {
        r = x;
        ch = s[2];
    }
}
// ====
// compileViaYul: also
// ----
// constructor(): 1, 2, 3, 4 ->
// r() -> 4
// ch() -> 3
