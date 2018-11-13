contract C {
    struct S { uint a; string b; }
    S x;
    function f() view public {
        S storage y = x;
        y;
    }
}
