contract C {
    function f() public {
        assembly {
            invalid()
        }
    }
}
// ----
// f() -> FAILURE
