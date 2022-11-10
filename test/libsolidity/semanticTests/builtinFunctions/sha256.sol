contract C {
    function f(int256 input) public returns (bytes32 sha256hash) {
        return sha256(abi.encodePacked(bytes32(uint256(input))));
    }
}
// ----
// f(int256): 4 -> 0xe38990d0c7fc009880a9c07c23842e886c6bbdc964ce6bdd5817ad357335ee6f
// f(int256): 5 -> 0x96de8fc8c256fa1e1556d41af431cace7dca68707c78dd88c3acab8b17164c47
// f(int256): -1 -> 0xaf9613760f72635fbdb44a5a0a63c39f12af30f950a6ee5c971be188e89c4051
