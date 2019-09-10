contract C {
    function f() pure public {
        // Periods are part of identifiers in assembly,
        // but not in Solidity. This tests that this scanner
        // setting is properly reset early enough.
        assembly { }
        C.f();
    }
}
// ----
