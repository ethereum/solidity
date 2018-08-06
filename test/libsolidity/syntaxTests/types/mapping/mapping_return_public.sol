contract C {
    function f() public pure returns (mapping(uint=>uint) storage m) {
    }
}
// ----
// TypeError: (51-80): Type is required to live outside storage.
// TypeError: (51-80): Internal or recursive type is not allowed for public or external functions.
