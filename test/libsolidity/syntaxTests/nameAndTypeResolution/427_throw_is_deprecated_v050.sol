pragma experimental "v0.5.0";
contract C {
    function f() pure public {
        throw;
    }
}
// ----
// SyntaxError: (82-87): "throw" is deprecated in favour of "revert()", "require()" and "assert()".
