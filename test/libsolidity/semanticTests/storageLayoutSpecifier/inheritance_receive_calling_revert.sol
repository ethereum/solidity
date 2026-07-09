contract A { receive() external payable virtual { revert("A"); } }
contract B is A { receive() external payable override virtual { revert("B"); } }
contract C1 is B layout at 2**256 - 2**42 { receive() external payable override virtual { revert("C"); } }
contract C2 is B { receive() external payable override virtual { revert("C"); } }
contract F {
    function decode(bytes memory data) internal returns (string memory) {
        assembly {
            // Shift the memory pointer forward by 4 bytes to skip the selector
            data := add(data, 4)
        }
        return abi.decode(data, (string));
    }
    function withSpecifier() public returns (string memory) {
        (bool success, bytes memory data) = payable(new C1()).call{value: 0}("");
        require(!success, "Must fail!");
        return decode(data);
    }
    function withoutSpecifier() public returns (string memory) {
        (bool success, bytes memory data) = payable(new C2()).call{value: 0}("");
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
