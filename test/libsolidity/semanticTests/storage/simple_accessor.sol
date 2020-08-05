contract test {
    uint256 public data;
    constructor() {
        data = 8;
    }
}
// ====
// compileViaYul: also
// ----
// data() -> 8
