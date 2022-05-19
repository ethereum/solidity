contract C {
    function f() public returns (uint x, uint y) {
        assembly {
            x := true
            y := false
        }
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 1, 0
