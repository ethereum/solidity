contract C {
    function f() pure public {
        uint super = 3;
        uint this = 4;
    }
}
// ----
// Warning 2319: (52-62): This declaration shadows a builtin symbol.
// Warning 2319: (76-85): This declaration shadows a builtin symbol.
// Warning 2072: (52-62): Unused local variable.
// Warning 2072: (76-85): Unused local variable.
