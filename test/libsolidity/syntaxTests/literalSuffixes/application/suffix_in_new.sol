function twice(uint x) pure suffix returns (uint) { return x * 2; }

contract C {
    function f() public pure returns (uint[] memory) {
        return new uint[](5 twice);
    }
}
