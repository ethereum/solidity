contract test {
    uint256 public data;
    constructor() public {
        data = 8;
    }
}
// ====
// compileViaYul: also
// ----
// data() -> 8
