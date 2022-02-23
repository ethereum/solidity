contract C {
    function h() public pure returns(uint[]) {}
}
// ----
// TypeError 6651: (50-56): Data location must be "memory" or "calldata" for return parameter in function, but none was given.
