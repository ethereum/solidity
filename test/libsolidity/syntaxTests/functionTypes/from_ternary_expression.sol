contract C {
    function f() public pure returns (uint x) {
        x = (true ? addmod : addmod)(3, 4, 5);
    }
}
// ----
// TypeError 9717: (81-87): Invalid mobile type in true expression.
// TypeError 3703: (90-96): Invalid mobile type in false expression.
