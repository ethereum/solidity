contract C {
    function keccak1() public pure returns (bytes32) {
		return keccak256("123");
    }
    function keccak2() public pure returns (bytes32) {
		bytes memory a = "123";
		return keccak256(a);
    }
}
// ====
// compileViaYul: only
// ----
// keccak1() -> 0x64e604787cbf194841e7b68d7cd28786f6c9a0a3ab9f8b0a0e87cb4387ab0107
// keccak2() -> 0x64e604787cbf194841e7b68d7cd28786f6c9a0a3ab9f8b0a0e87cb4387ab0107
