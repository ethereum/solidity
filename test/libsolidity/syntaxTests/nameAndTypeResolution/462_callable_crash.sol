contract C {
    struct S { uint a; bool x; }
    S public s;
    constructor() public {
        3({a: 1, x: true});
    }
}
// ----
// TypeError: (97-115): Type is not callable
