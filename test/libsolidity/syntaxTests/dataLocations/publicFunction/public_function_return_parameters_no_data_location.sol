contract C {
    function h() public pure returns(uint[]) {}
}
// ----
// TypeError: (50-56): Storage location must be one of "memory" for parameter in public function, but none was given.
