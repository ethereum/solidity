pragma experimental ABIEncoderV2;
library L {
    function f(mapping(uint => uint)[2] memory a) external pure returns (mapping(uint => uint)[2] memory) {}
}
// ----
// TypeError 4061: (61-94): Type mapping(uint256 => uint256)[2] is only valid in storage because it contains a (nested) mapping.
