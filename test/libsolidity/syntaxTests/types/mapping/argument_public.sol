contract C {
    function f(mapping(uint => uint) storage) public pure {
    }
}
// ----
// TypeError 6651: (28-57='mapping(uint => uint) storage'): Data location must be "memory" or "calldata" for parameter in function, but "storage" was given.
