contract C {
    function h() public pure returns(uint[] transient) {}
}
// ----
// TypeError 6651: (50-66): Data location must be "memory" or "calldata" for return parameter in function, but none was given.
