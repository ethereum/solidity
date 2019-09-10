contract C {
    function f() public pure returns (uint32 x) {
        uint32 a;
        uint32 b;
        uint32 c;
        assembly {
            a := 1
            b := 2
            c := 3
        }
        x = a + b + c;
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 6
