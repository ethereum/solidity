contract Test {
    bytes32 constant public b = "abcdefghijklmnopq";
    string constant public x = "abefghijklmnopqabcdefghijklmnopqabcdefghijklmnopqabca";

    constructor() {
        string memory xx = x;
        bytes32 bb = b;
    }
    function getB() public returns (bytes32) { return b; }
    function getX() public returns (string memory) { return x; }
    function getX2() public returns (string memory r) { r = x; }
    function unused() public returns (uint) {
        "unusedunusedunusedunusedunusedunusedunusedunusedunusedunusedunusedunused";
        return 2;
    }
}
// ----
// b() -> 0x6162636465666768696a6b6c6d6e6f7071000000000000000000000000000000
// x() -> 0x20, 0x35, 0x616265666768696a6b6c6d6e6f70716162636465666768696a6b6c6d6e6f7071, 44048183304486788312148433451363384677562177293131179093971701692629931524096
// getB() -> 0x6162636465666768696a6b6c6d6e6f7071000000000000000000000000000000
// getX() -> 0x20, 0x35, 0x616265666768696a6b6c6d6e6f70716162636465666768696a6b6c6d6e6f7071, 44048183304486788312148433451363384677562177293131179093971701692629931524096
// getX2() -> 0x20, 0x35, 0x616265666768696a6b6c6d6e6f70716162636465666768696a6b6c6d6e6f7071, 44048183304486788312148433451363384677562177293131179093971701692629931524096
// unused() -> 2
