contract A {
    function f() public returns(bool) {
        (bool success, ) = address(this).call("");
        return success;
    }
}

// ----
// f() -> 0
