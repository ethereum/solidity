contract C {
    function f() public {
        assert(false);
    }

    function g(bool val) public returns(bool) {
        assert(val == true);
        return true;
    }

    function h(bool val) public returns(bool) {
        require(val);
        return true;
    }
}

// ====
// compileViaYul: also
// ----
// f() -> FAILURE
// g(bool): 0 -> FAILURE
// g(bool): 1 -> 1
// h(bool): 0 -> FAILURE
// h(bool): 1 -> 1
