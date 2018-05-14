contract C {
    struct S { bool f; }
    S s;
    function f() internal pure returns (S storage) {
        throw;
    }
}
// ----
// Warning: (108-113): "throw" is deprecated in favour of "revert()", "require()" and "assert()".
