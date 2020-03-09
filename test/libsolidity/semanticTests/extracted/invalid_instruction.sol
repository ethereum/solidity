contract C {
    function f() public {
        assembly {
            invalid()
        }
    }
}

// ====
// compileViaYul: also
// ----
// f() -> FAILURE
