contract C {
    bytes s;
    function f(bytes calldata c, string calldata c1) public {
        bytes memory a;
        bytes16 b;
        uint8[] memory num;
        bytes1[] memory m;
        bytes memory d = bytes.concat(a, b, c, num, s, "abc", m, c1, bytes(c1));
    }
}
// ----
// TypeError 8015: (233-236): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint8[] memory provided.
// TypeError 8015: (248-249): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but bytes1[] memory provided.
// TypeError 8015: (251-253): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but string calldata provided.
