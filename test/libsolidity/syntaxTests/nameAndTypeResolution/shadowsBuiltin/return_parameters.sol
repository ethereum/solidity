contract C {
    function f() pure public returns (uint require) {
        require = 2;
    }
}
// ----
// Warning: (51-63): This declaration shadows a builtin symbol.
