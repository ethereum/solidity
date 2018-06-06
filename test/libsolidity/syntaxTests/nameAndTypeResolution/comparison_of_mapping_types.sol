contract C {
    mapping(uint => uint) x;
    function f() public returns (bool ret) {
        var y = x;
        return x == y;
    }
}
// ----
// Warning: (95-100): Use of the "var" keyword is deprecated.
// TypeError: (121-127): Operator == not compatible with types mapping(uint256 => uint256) and mapping(uint256 => uint256)
