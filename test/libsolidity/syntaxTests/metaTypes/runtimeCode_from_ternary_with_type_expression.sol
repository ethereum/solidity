contract A {
    function f() public {}
}

contract B {
    function g() public {}
}

contract C {
    function ab(bool getA) pure public returns (bytes memory) {
        return (getA ? type(A) : type(B)).runtimeCode;
    }
}
// ----
// TypeError 9717: (186-193): Invalid mobile type in true expression.
// TypeError 3703: (196-203): Invalid mobile type in false expression.
