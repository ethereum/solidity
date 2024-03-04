struct S {
    uint x;
    uint y;
    uint z;
}

contract C {
    function f(bool c) public pure {
        S memory s = (c ? S : S)(0, 1, 2);
    }
}
// ----
// TypeError 9717: (126-127): Invalid mobile type in true expression.
// TypeError 3703: (130-131): Invalid mobile type in false expression.
