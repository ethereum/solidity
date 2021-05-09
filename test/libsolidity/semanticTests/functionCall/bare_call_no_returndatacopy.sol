contract C {
    function f() public returns (bool) {
        (bool success, ) = address(1).call("");
        return success;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> true
