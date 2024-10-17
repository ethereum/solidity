function erc7201(bytes memory id) pure returns (uint256) {
    return uint256(
        keccak256(bytes.concat(bytes32(uint256(keccak256(id)) - 1))) &
        ~bytes32(uint256(0xff))
    );
}
contract C layout at erc7201("C") { }
// ----
