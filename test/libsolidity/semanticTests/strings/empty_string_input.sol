contract C {
    function f() public pure returns (string memory) {
        return "";
    }
    function g(string calldata msg) public pure returns (string memory) {
        return msg;
    }
    function h(string calldata msg, uint256 v) public pure returns (string memory, uint256) {
        return (msg, v);
    }
    // Adjusting order of input/output intentionally.
    function i(string calldata msg1, uint256 v, string calldata msg2) public pure returns (string memory, string memory, uint256) {
        return (msg1, msg2, v);
    }
    function j(string calldata msg1, uint256 v) public pure returns (string memory, string memory, uint256) {
        return (msg1, "", v);
    }
}
// ----
// f() -> 0x20, 0
// g(string): 0x20, 0, "" -> 0x20, 0
// g(string): 0x20, 0 -> 0x20, 0
// h(string,uint256): 0x40, 0x888, 0, "" -> 0x40, 0x0888, 0
// h(string,uint256): 0x40, 0x888, 0 -> 0x40, 0x0888, 0
// i(string,uint256,string): 0x60, 0x888, 0x60, 0, "" -> 0x60, 0x80, 0x0888, 0, 0
// i(string,uint256,string): 0x60, 0x888, 0x60, 0 -> 0x60, 0x80, 0x0888, 0, 0
// j(string,uint256): 0x40, 0x888, 0, "" -> 0x60, 0x80, 0x0888, 0, 0
// j(string,uint256): 0x40, 0x888, 0 -> 0x60, 0x80, 0x0888, 0, 0
