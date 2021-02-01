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
// gas ir: 100000000
// gas irOptimized: 100000000
// gas legacy: 100000000
// gas legacyOptimized: 100000000
