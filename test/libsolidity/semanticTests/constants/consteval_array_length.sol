contract C {
    uint constant a = 12;
    uint constant b = 10;

    function f() public pure returns (uint, uint) {
        uint[(a / b) * b] memory x;
        return (x.length, (a / b) * b);
    }
}
// ====
// compileViaYul: true
// ----
// constructor() ->
// gas ir: 240187
// gas irOptimized: 105929
// f() -> 0x0a, 0x0a
