contract C {
    function f() public pure returns (mapping(uint=>uint) memory m) {
    }
}
// ----
// TypeError: (51-79): Type is required to live outside storage.
// TypeError: (51-79): Internal or recursive type is not allowed for public or external functions.
