contract C {
    function f() public {
        assembly {
            invalid()
        }
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> FAILURE
