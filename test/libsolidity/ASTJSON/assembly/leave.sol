contract C {
    function l() public {
        assembly { function f() { leave } }
    }
}

// ----
