contract C {
    function f() public {
        assembly {
            invalid()
        }
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> FAILURE
