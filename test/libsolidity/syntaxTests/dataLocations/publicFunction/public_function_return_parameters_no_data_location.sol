contract C {
    function h() public pure returns(uint[]) {}
}
// ----
// TypeError: (50-56): Location must be specified as "memory" for parameters in publicly visible functions.
