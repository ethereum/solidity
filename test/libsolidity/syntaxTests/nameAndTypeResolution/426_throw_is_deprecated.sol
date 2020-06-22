contract C {
    function f() pure public {
        throw;
    }
}
// ----
// SyntaxError 4538: (52-57): "throw" is deprecated in favour of "revert()", "require()" and "assert()".
