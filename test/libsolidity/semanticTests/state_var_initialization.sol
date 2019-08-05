contract C {
    uint public i = 1;
    uint public k = 2;

    constructor() public {
        i = i + i;
        k = k - i;
    }
}
// ====
// compileViaYul: also
// ----
// i() -> 2
// k() -> 0
