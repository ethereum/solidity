// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0.0;

contract Error1 {
  constructor() {
    balances[tx.origin] = ; // missing RHS.
  }

  // This function parses properly
  function five() public view returns(uint) {
    return 5;
  }
}
