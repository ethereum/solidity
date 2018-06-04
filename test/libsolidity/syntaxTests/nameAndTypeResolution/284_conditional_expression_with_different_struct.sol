contract C {
    struct s1 {
        uint x;
    }
    struct s2 {
        uint x;
    }
    function f() public {
        s1 memory x;
        s2 memory y;
        true ? x : y;
    }
}
// ----
// TypeError: (165-177): True expression's type struct C.s1 memory doesn't match false expression's type struct C.s2 memory.
