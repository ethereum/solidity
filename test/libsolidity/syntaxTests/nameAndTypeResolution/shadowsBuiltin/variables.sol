contract C {
    function f() pure public {
        uint msg;
        msg;
    }
}
// ----
// Warning 2319: (52-60='uint msg'): This declaration shadows a builtin symbol.
