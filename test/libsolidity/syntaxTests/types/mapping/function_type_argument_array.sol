contract test {
    function f(mapping(uint => uint)[2] memory b) internal {
    }
}
// ----
// TypeError 4061: (31-64): Type mapping(uint256 => uint256)[2] memory is only valid in storage because it contains a (nested) mapping.
