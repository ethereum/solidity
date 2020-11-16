contract C {
    uint constant a = 12;
    uint constant b = 10;

    function f() public pure returns (uint) {
        uint[(a / b) * b] memory x;
        return x.length;
    }
}
// ====
// compileViaYul: true
// ----
// constructor() ->
// f() -> 10
