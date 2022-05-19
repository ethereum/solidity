contract Test {
    string s;
    bytes b;

    function f(string memory _s, uint256 n) public returns (bytes1) {
        b = bytes(_s);
        s = string(b);
        return bytes(s)[n];
    }

    function l() public returns (uint256) {
        return bytes(s).length;
    }
}
// ----
// f(string,uint256): 0x40, 0x02, 0x06, "abcdef" -> "c"
// l() -> 0x06
