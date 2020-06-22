contract C {
    function f(mapping(uint => uint)[] storage) public pure {
    }
}
// ----
// TypeError 6651: (28-59): Data location must be "memory" or "calldata" for parameter in function, but "storage" was given.
