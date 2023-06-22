library L {
    function f() public pure { }
}

contract C {
    function g(bool c) public {
        (c ? L : L).f();
    }
}
// ----
// TypeError 9717: (106-107): Invalid mobile type in true expression.
// TypeError 3703: (110-111): Invalid mobile type in false expression.
