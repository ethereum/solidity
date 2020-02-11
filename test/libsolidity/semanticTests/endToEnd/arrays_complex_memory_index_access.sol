contract Test {
    function set(uint24[3][] memory _data, uint a, uint b) public returns(uint l, uint e) {
        l = _data.length;
        e = _data[a][b];
    }
}

// ====
// compileViaYul: also
// ----
set(uint24[3][], uint256, uint256): "96, 3, 2, 6, ?"
