contract C {
    function f() public pure { }
    function g(bool c) public {
        (c ? C : C).f();
    }
}
// ----
// TypeError 9717: (91-92): Invalid mobile type in true expression.
// TypeError 3703: (95-96): Invalid mobile type in false expression.
