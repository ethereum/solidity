contract C {
    function f() public returns(uint, uint[] memory, uint) {
        return (7, new uint[](2), 8);
    }

    function g() public returns(uint, uint) {
        // Previous implementation "moved" b to the second place and did not skip.
        (uint a, , uint b) = this.f();
        return (a, b);
    }
}

// ----
// g() -> 7, 8
