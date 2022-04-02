contract C {
    struct S { uint a; bool x; }
    S public s;
    constructor() {
        3({a: 1, x: true});
    }
}
// ----
// TypeError 5704: (90-108='3({a: 1, x: true})'): Type is not callable
