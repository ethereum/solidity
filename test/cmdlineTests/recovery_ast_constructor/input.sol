pragma solidity >=0.0.0;

contract Error1 {
  constructor() public {
    balances[tx.origin] = ; // missing RHS.
  }

  // Without error recovery we stop due to the above error.
  // Error recovery however recovers at the above ';'
  // There should be an AST for the above, albeit with error
  // nodes.

  // This function parses properly and should give AST info.
  function five() public view returns(uint) {
    return 5;
  }
}
