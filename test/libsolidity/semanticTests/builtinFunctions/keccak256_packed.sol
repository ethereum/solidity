contract C {
    function f(int256 _input) public returns (bytes32 hash) {
        uint24 b = 65536;
        uint c = 256;
        bytes32 input = bytes32(uint256(_input));
        return keccak256(abi.encodePacked(uint8(8), input, b, input, c));
    }
}
// ----
// f(int256): 4 -> 0xd270285b9966fefc715561efcd09d5b6a8deb15596f7c53cb4a1bb73aa55ac3a
// f(int256): 5 -> 0xf2f92566c5653600c1e527a7073e5d881576d12bb51887c0b8f3e1f81865b03d
// f(int256): -1 -> 0xbc78b45e0db67af5af72e4ab62757c67aefa7388cdf0c4e74f8b5fe9dd5d9d13
