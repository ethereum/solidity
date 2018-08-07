contract C {
    function h() public pure returns(uint[]) {}
}
// ----
// TypeError: (50-56): Data location must be "memory" for return parameter in function, but none was given.
