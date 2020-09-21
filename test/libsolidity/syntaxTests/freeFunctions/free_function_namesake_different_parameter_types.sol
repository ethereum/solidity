function g() pure returns (uint) { return 1; }
function g() pure returns (string memory) { return "1"; }
contract C {
  function foo() public pure returns (uint) {
    string memory s = g();
    return 100/g();
  }
}
// ----
// DeclarationError 1686: (0-46): Function with same name and parameter types defined twice.
// TypeError 9574: (168-189): Type uint256 is not implicitly convertible to expected type string memory.
