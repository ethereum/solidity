contract C {
    function f() pure public {
        uint x; uint y;
        assembly { x, y := 7 }
    }
}
// ----
// DeclarationError 8678: (87-96): Variable count for assignment to "x, y" does not match number of values (2 vs. 1)
