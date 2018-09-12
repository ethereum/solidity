contract C {
    function f(mapping(uint => uint)[] storage) external pure {
    }
}
// ----
// TypeError: (28-59): Data location must be "calldata" for parameter in external function, but "storage" was given.
