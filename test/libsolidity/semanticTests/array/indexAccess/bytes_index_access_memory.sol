contract Main {
    function f(bytes memory _s1, uint i1, uint i2, uint i3) public returns (bytes1 c1, bytes1 c2, bytes1 c3) {
        c1 = _s1[i1];
        c2 = intern(_s1, i2);
        c3 = internIndirect(_s1)[i3];
    }
    function intern(bytes memory _s1, uint i) public returns (bytes1 c) {
        return _s1[i];
    }
    function internIndirect(bytes memory _s1) public returns (bytes memory) {
        return _s1;
    }
}
// ----
// f(bytes,uint256,uint256,uint256): 0x80, 3, 4, 5, 78, "abcdefghijklmnopqrstuvwxyzabcdef", "ghijklmnopqrstuvwxyzabcdefghijkl", "mnopqrstuvwxyz" -> "d", "e", "f"
