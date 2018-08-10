contract C {
    function f(mapping(uint => uint) storage) external pure {
    }
}
// ----
// TypeError: (28-49): Type is required to live outside storage.
// TypeError: (28-49): Internal or recursive type is not allowed for public or external functions.
