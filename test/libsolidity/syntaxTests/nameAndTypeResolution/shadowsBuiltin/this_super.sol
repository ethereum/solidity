contract C {
    function f() pure public {
        uint super = 3;
        uint this = 4;
    }
}
// ----
// DeclarationError 3726: (52-62): The name "super" is reserved.
// DeclarationError 3726: (76-85): The name "this" is reserved.
// Warning 2319: (52-62): This declaration shadows a builtin symbol.
// Warning 2319: (76-85): This declaration shadows a builtin symbol.
