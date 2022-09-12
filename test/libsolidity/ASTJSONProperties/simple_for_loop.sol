function f() pure {
    /// @custom:test { "isSimpleCounterLoop": true }
    for (uint i = 0; i < 32; ++i) {
    }
    /// @custom:test { "isSimpleCounterLoop": true }
    for (uint i = 0; i < 32; i++) {
    }
    /// @custom:test { "isSimpleCounterLoop": true }
    for (uint i = 0; i < i; ++i) {
    }
    /// @custom:test { "isSimpleCounterLoop": false }
    for (uint i = 0; 0 < i; ++i) {
    }
}

// ----
