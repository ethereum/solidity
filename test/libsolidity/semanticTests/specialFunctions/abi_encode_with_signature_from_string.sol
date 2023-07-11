contract C {
    function f() public pure returns (bytes memory r1, bytes memory r2) {
        string memory x = "my_signature";
        r1 = abi.encodeWithSignature("my_signature", 1);
        r2 = abi.encodeWithSignature(x, 1);
        assert(r1.length == r2.length);
        for (uint i = 0; i < r1.length; i++)
            assert(r1[i] == r2[i]);
    }
}
// ----
// f() -> 0x40, 0xa0, 0x24, -813742827273327954027712588510533233455028711326166692885570228492575965184, 26959946667150639794667015087019630673637144422540572481103610249216, 0x24, -813742827273327954027712588510533233455028711326166692885570228492575965184, 26959946667150639794667015087019630673637144422540572481103610249216
