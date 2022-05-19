contract C {
    function f() public {
        assembly {
            invalid()
        }
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> FAILURE
