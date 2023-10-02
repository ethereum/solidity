contract C {
    function f() public pure { }
    function g() public pure { }
}

contract A {
    function test(bool b) public returns(bytes4) {
        (b ? C.f : C.g).selector;
    }
}
// ----
// TypeError 9717: (159-162): Invalid mobile type in true expression.
// TypeError 3703: (165-168): Invalid mobile type in false expression.
