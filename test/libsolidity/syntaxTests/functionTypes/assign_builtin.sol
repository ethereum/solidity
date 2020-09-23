contract C {
  function f() public {
     function (uint) view returns (bytes32) _blockhash = blockhash;
  }
}
// ----
// TypeError 9574: (42-103): Type function (uint256) view returns (bytes32) is not implicitly convertible to expected type function (uint256) view returns (bytes32). Special functions can not be converted to function types.
