contract C {
    function f() external pure returns (mapping(uint=>uint) storage m) {
    }
}
// ----
// TypeError: (53-82): Type is required to live outside storage.
// TypeError: (53-82): Internal or recursive type is not allowed for public or external functions.
