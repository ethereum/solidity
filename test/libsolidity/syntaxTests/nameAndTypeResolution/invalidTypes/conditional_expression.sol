contract C {
    function o(byte) public pure {}
    function f() public {
        o(true ? 99**99 : 99);
        o(true ? 99 : 99**99);

        o(true ? 99**99 : 99**99);
    }
}
// ----
// TypeError 9717: (92-98): Invalid mobile type in true expression.
// TypeError 9553: (85-103): Invalid type for argument in function call. Invalid implicit conversion from uint8 to bytes1 requested.
// TypeError 3703: (128-134): Invalid mobile type in false expression.
// TypeError 9553: (116-134): Invalid type for argument in function call. Invalid implicit conversion from uint8 to bytes1 requested.
// TypeError 9717: (155-161): Invalid mobile type in true expression.
// TypeError 3703: (164-170): Invalid mobile type in false expression.
