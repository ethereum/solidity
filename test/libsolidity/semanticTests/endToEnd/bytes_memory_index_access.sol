contract Test {
    function set(bytes memory _data, uint i) public returns(uint l, byte c) {
        l = _data.length;
        c = _data[i];
    }
}

// ====
// compileViaYul: also
// ----
set(bytes, uint256): "64, 3, 8, abcdefgh"
