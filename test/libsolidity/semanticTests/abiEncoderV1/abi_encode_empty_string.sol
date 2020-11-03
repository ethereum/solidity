contract C {
    function f1() public returns (bytes memory) {
        return abi.encode("");
    }
    function f2(string calldata msg) public returns (bytes memory) {
        return abi.encode(msg);
    }
    function g1() public returns (bytes memory) {
        return abi.encodePacked("");
    }
    function g2(string calldata msg) public returns (bytes memory) {
        return abi.encodePacked(msg);
    }
    function h1() public returns (bytes memory) {
        return abi.encodeWithSelector(0x00000001, "");
    }
    function h2(string calldata msg) public returns (bytes memory) {
        return abi.encodeWithSelector(0x00000001, msg);
    }
}

// ====
// ABIEncoderV1Only: true
// compileViaYul: false
// ----
// f1() -> 0x20, 0x40, 0x20, 0
// f2(string): 0x20, 0 -> 0x20, 0x40, 0x20, 0
// f2(string): 0x20, 0, 0 -> 0x20, 0x40, 0x20, 0
// g1() -> 32, 0
// g2(string): 0x20, 0 -> 0x20, 0
// g2(string): 0x20, 0, 0 -> 0x20, 0
// h1() -> 0x20, 0x44, 26959946667150639794667015087019630673637144422540572481103610249216, 862718293348820473429344482784628181556388621521298319395315527974912, 0
// h2(string): 0x20, 0 -> 0x20, 0x44, 26959946667150639794667015087019630673637144422540572481103610249216, 862718293348820473429344482784628181556388621521298319395315527974912, 0
// h2(string): 0x20, 0, 0 -> 0x20, 0x44, 26959946667150639794667015087019630673637144422540572481103610249216, 862718293348820473429344482784628181556388621521298319395315527974912, 0
