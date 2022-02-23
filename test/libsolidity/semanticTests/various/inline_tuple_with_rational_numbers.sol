contract c {
    function f() public returns (int8) {
        int8[5] memory foo3 = [int8(1), -1, 0, 0, 0];
        return foo3[0];
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 1
