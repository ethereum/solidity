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
// f() -> 
// f():"" -> ""
// g(bool): false -> 
// g(bool):"0" -> ""
// g(bool): true -> true
// g(bool):"1" -> "1"
// h(bool): false -> 
// h(bool):"0" -> ""
// h(bool): true -> true
// h(bool):"1" -> "1"
