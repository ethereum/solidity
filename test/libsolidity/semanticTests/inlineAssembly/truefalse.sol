contract C {
    function f() public returns (uint x, uint y) {
        assembly {
            x := true
            y := false
        }
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 1, 0
