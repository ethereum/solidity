contract A { fallback() external virtual { revert("A"); } }
contract B is A { fallback() external override virtual { revert("B"); } }
contract C1 is B layout at 2**256 - 2**42 { fallback() external override virtual { revert("C"); } }
contract C2 is B { fallback() external override virtual { revert("C"); } }
contract F {
    function decode(bytes memory data) internal returns (string memory) {
        assembly {
            // Shift the memory pointer forward by 4 bytes to skip the selector
            data := add(data, 4)
        }
        return abi.decode(data, (string));
    }
    function withSpecifier() public returns (string memory) {
        (bool success, bytes memory data) = address(new C1()).call(hex"beee");
        require(!success, "Must fail!");
        return decode(data);
    }
    function withoutSpecifier() public returns (string memory) {
        (bool success, bytes memory data) = address(new C2()).call(hex"beee");
        require(!success, "Must fail!");
        return decode(data);
    }
}
// ====
// EVMVersion: >homestead
// targetContract: F
// ----
// withSpecifier() -> 0x20, 1, "C"
// withoutSpecifier() -> 0x20, 1, "C"
