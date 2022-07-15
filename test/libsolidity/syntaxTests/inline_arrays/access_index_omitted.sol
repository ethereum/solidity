contract C {
    function f() public {
        uint i = [0, 1, 2][];
    }
}
// ----
// TypeError 5093: (56-67): Index expression cannot be omitted.
