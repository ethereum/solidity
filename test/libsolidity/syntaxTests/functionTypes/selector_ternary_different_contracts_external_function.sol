contract C {
    function f() external pure { }
}

contract D {
    function g() external pure { }
}

contract A {
    function test(bool b) public returns(bytes4) {
        (b ? C.f : D.g).selector;
    }
}
// ----
// TypeError 9717: (179-182): Invalid mobile type in true expression.
// TypeError 3703: (185-188): Invalid mobile type in false expression.
