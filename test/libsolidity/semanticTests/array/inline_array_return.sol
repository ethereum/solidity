contract C {
    uint8[] tester;

    function f() public returns (uint8[5] memory) {
        return ([1, 2, 3, 4, 5]);
    }

    function test() public returns (uint8, uint8, uint8, uint8, uint8) {
        tester = f();
        return (tester[0], tester[1], tester[2], tester[3], tester[4]);
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> 1, 2, 3, 4, 5
