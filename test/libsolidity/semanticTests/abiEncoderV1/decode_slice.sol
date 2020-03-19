contract C {
    function f(uint256 a, uint256 b) external returns (uint256 c, uint256 d, uint256 e, uint256 f) {
        (c, d) = abi.decode(msg.data[4:], (uint256, uint256));
        e = abi.decode(msg.data[4 : 4 + 32], (uint256));
        f = abi.decode(msg.data[4 + 32 : 4 + 32 + 32], (uint256));
    }
}
// ----
// f(uint256,uint256): 42, 23 -> 42, 23, 42, 23
