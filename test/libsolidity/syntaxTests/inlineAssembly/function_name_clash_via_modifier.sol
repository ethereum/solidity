contract C {
    uint y;

    modifier m() {
        uint t;
        assembly {
            function f() -> x { x := 8 }
            t := f()
        }
        y = t;
        _;
    }

    function f() m m public returns (uint r) {
        assembly { function f() -> x { x := 1 } r := f() }
    }
    function g() m m public returns (uint r) {
        assembly { function f() -> x { x := 2 } r := f() }
    }
}
// ----
// DeclarationError 3859: (92-120): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError 3859: (251-279): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError 3859: (363-391): This declaration shadows a declaration outside the inline assembly block.
