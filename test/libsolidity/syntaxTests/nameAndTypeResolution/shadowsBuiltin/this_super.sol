contract C {
    function f() pure public {
        uint super = 3;
        uint this = 4;
    }
}
// ----
// Warning: (52-62): This declaration shadows a builtin symbol.
// Warning: (76-85): This declaration shadows a builtin symbol.
// Warning: (52-62): Unused local variable.
// Warning: (76-85): Unused local variable.
