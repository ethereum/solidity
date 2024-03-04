contract A {
}

contract B {
}

contract C {
    function f(bool getA) public returns (bytes memory) {
        return (getA ? type(A) : type(B)).runtimeCode;
    }
}
// ----
// TypeError 9717: (126-133): Invalid mobile type in true expression.
// TypeError 3703: (136-143): Invalid mobile type in false expression.
