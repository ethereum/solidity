pragma abicoder v2;

struct T {
    bytes x;
    uint[3] y;
}

contract E {
    function f(bool a, T calldata b, bytes32[2] calldata c)
        public
        returns (bool, T calldata, bytes32[2] calldata)
    {
        return (a, b, c);
    }
}
// ----
// f(bool,(bytes,uint256[3]),bytes32[2]): 1, 0x80, "a", "b", 0x80, 11, 12, 13, 4, "abcd" -> 1, 0x80, "a", "b", 0x80, 11, 12, 13, 4, "abcd"
