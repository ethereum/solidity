contract C {
    function o(byte) public pure {}
    function f() public {
        o(true ? 99**99 : 99);
        o(true ? 99 : 99**99);

        o(true ? 99**99 : 99**99);
    }
}
// ----
// TypeError: (92-98): Invalid mobile type in true expression.
// TypeError: (85-103): Invalid type for argument in function call. Invalid implicit conversion from uint8 to bytes1 requested.
// TypeError: (128-134): Invalid mobile type in false expression.
// TypeError: (116-134): Invalid type for argument in function call. Invalid implicit conversion from uint8 to bytes1 requested.
// TypeError: (155-161): Invalid mobile type in true expression.
// TypeError: (164-170): Invalid mobile type in false expression.
