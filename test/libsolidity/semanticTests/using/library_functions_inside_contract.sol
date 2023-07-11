library L {
    function externalFunction(uint a) external pure returns (uint) { return a * 1; }
    function publicFunction(uint b) public pure returns (uint) { return b * 2; }
    function internalFunction(uint c) internal pure returns (uint) { return c * 3; }
}

contract C {
    using {L.externalFunction} for uint;
    using {L.publicFunction} for uint;
    using {L.internalFunction} for uint;

    function f() public pure returns (uint) {
        uint x = 1;
        return x.externalFunction();
    }

    function g() public pure returns (uint) {
        uint x = 1;
        return x.publicFunction();
    }

    function h() public pure returns (uint) {
        uint x = 1;
        return x.internalFunction();
    }
}
// ----
// library: L
// f() -> 1
// g() -> 2
// h() -> 3
