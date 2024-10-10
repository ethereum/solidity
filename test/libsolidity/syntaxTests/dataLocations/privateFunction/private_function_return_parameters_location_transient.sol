contract C {
    function f() private pure returns (uint[] transient) {}
}
// ----
// TypeError 6651: (52-68): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
