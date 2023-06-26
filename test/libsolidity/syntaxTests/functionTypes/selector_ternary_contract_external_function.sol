contract C {
    function f() external pure { }
    function g() external pure { }
    function test(bool b) public returns(bytes4) {
        (b ? C.f : C.g).selector;
    }
}
// ----
// TypeError 9717: (147-150): Invalid mobile type in true expression.
// TypeError 3703: (153-156): Invalid mobile type in false expression.
