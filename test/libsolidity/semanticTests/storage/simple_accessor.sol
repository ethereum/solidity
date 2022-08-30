contract test {
    uint256 public data;
    constructor() {
        data = 8;
    }
}
// ====
// compileToEwasm: also
// ----
// data() -> 8
