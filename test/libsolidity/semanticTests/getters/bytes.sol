contract C {
    bytes public b;
    constructor() {
        b = "abc";
    }
}
// ====
// compileToEwasm: also
// ----
// b() -> 0x20, 0x03, 0x6162630000000000000000000000000000000000000000000000000000000000
