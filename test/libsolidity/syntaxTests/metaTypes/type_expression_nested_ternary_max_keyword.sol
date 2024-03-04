contract C {
    function f(bool b) public pure returns (uint) {
        return type(b ? uint : uint).max;
    }
}
// ----
// TypeError 9717: (89-93): Invalid mobile type in true expression.
// TypeError 3703: (96-100): Invalid mobile type in false expression.
