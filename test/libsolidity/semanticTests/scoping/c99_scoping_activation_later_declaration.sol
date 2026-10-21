contract C {
    uint x = 1;

    function f() public view returns (uint a, uint b, uint c) {
        { uint x = 2; a = x; } // Refers to the block's x
        b = x; // Refers to the state variable, the local x is not visible yet
        uint x = 3;
        c = x;
    }

    function g() public view returns (uint a, uint b, uint c) {
        for (uint x = 4; x < 5; ++x) a = x;
        b = x;
        uint x = 5;
        c = x;
    }

    function h() public returns (uint a, uint b, uint c) {
        try this.v() returns (uint x) { a = x; } catch {}
        b = x;
        uint x = 7;
        c = x;
    }

    function v() external pure returns (uint) { return 6; }
}
// ----
// f() -> 2, 1, 3
// g() -> 4, 1, 5
// h() -> 6, 1, 7
