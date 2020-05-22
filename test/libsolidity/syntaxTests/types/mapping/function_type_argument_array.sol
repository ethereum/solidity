contract test {
    function f(mapping(uint => uint)[2] memory b) internal {
    }
}
// ----
// TypeError: (31-64): Type mapping(uint256 => uint256)[2] is only valid in storage.
