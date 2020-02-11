contract Test {
    string s;
    bytes b;

    function f(string memory _s, uint n) public returns(byte) {
        b = bytes(_s);
        s = string(b);
        return bytes(s)[n];
    }

    function l() public returns(uint) {
        return bytes(s).length;
    }
}

// ----
// callContractFunction( "f(string,uint256): 0x40), 2), 6), string("abcdef")  -> "c"
// f(string,uint256):"64, 2, 6, abcdef" -> "c"
// l() -> 6
// l():"" -> "6"
