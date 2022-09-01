contract Test {
    mapping(uint=>uint) data;
    function stat() public returns (uint[5] memory)
    {
        data[2] = 3; // make sure to use some memory
    }
    function dyn() public returns (uint[] memory) { stat(); }
    function nested() public returns (uint[3][] memory) { stat(); }
    function nestedStat() public returns (uint[3][7] memory) { stat(); }
}
// ----
// stat() -> 0, 0, 0, 0, 0
// dyn() -> 0x20, 0
// nested() -> 0x20, 0
// nestedStat() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
