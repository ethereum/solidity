// SPDX-License-Identifier: Apache-2.0

import "./contract.sol";

contract C {
  using Contract for address;
  using Contract for address payable;
  function f(address x) public returns (bytes memory) {
    if (x.codesize() != 0 && x.exists() && x.hasCode()) {
      return x.getCode();
    }
    address payable y = payable(x);
    y.transfer_(55);
  }
  function g(address payable x) public returns (bytes memory) {
    if (x.codesize() != 0 && x.exists() && x.hasCode()) {
      return x.getCode();
    }
    address payable y = payable(x);
    y.transfer_(55);
  }
}
