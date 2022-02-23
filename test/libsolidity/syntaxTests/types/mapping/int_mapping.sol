library L {
  function f(mapping(uint=>uint) memory) public {}
}
// ----
// TypeError 4061: (25-51): Type mapping(uint256 => uint256) is only valid in storage because it contains a (nested) mapping.
