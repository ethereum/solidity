contract C {
    function f() public {
        assert(false);
    }

    function g(bool val) public returns (bool) {
        assert(val == true);
        return true;
    }

    function h(bool val) public returns (bool) {
        require(val);
        return true;
    }
}

// ====
// compileViaYul: also
// ----
// f() -> FAILURE
// g(bool): false -> FAILURE
// g(bool): true -> true
// h(bool): false -> FAILURE
// h(bool): true -> true
