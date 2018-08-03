contract C {
    function h() public pure returns(uint[]) {}
}
// ----
// TypeError: (50-56): Storage location must be "memory" for parameter in public function, but none was given.
