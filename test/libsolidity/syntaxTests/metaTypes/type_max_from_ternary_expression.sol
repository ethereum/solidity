contract C {
    function max(bool isUint) public returns (uint8) {
        return (isUint ? type(uint8) : type(int8)).max;
    }
}
// ----
// TypeError 9717: (93-104): Invalid mobile type in true expression.
// TypeError 3703: (107-117): Invalid mobile type in false expression.
