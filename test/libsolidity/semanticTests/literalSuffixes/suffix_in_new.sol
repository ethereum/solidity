function twice(uint x) pure suffix returns (uint) { return x * 2; }

contract C {
    function run() public pure returns (uint[] memory) {
        return new uint[](5 twice);
    }
}
// ----
// run() -> 0x20, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
