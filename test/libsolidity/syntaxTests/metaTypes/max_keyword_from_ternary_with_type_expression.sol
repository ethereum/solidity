contract C {
    function max(bool isUint) pure public returns (uint8) {
        return (isUint ? type(uint8) : type(int8)).max;
    }
}
// ----
// TypeError 9717: (98-109): Invalid mobile type in true expression.
// TypeError 3703: (112-122): Invalid mobile type in false expression.
