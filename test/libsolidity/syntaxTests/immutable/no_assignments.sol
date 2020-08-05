contract C {
    uint immutable x;
    constructor() {
        x = 0;
        while (true)
        {}
    }
    function f() external view returns(uint) { return x; }
}
// ====
// optimize-yul: true
// ----
// CodeGenerationError 1284: Some immutables were read from but never assigned, possibly because of optimization.
