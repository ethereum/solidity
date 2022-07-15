contract C {
    function f() external pure {
        string[2] memory a1 = [string(bytes(hex'74000001')), string(bytes(hex'c0a80101'))];
        bytes[2] memory a2 = [bytes(hex'74000001'), bytes(hex'c0a80101')];
    }
}
// ----
// Warning 2072: (54-73): Unused local variable.
// Warning 2072: (146-164): Unused local variable.
