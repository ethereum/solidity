contract C {
    mapping(uint => uint) x;
    function f() public returns (bool ret) {
        mapping(uint => uint) storage y = x;
        return x == y;
    }
}
// ----
// TypeError: (147-153): Operator == not compatible with types mapping(uint256 => uint256) and mapping(uint256 => uint256)
