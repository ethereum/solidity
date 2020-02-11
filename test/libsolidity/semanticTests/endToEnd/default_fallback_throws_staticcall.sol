contract A {
    function f() public returns(bool) {
        (bool success, bytes memory data) = address(this).staticcall("");
        assert(data.length == 0);
        return success;
    }
}

// ----
// f() -> 0
