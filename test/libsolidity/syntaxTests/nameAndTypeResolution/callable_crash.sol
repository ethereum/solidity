contract C {
    struct S { uint a; bool x; }
    S public s;
    function C() public {
        3({a: 1, x: true});
    }
}
// ----
// Warning: (66-121): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (96-114): Type is not callable
