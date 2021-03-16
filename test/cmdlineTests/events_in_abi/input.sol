// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
library L {
  event e1(uint b);
}

function f() {
  emit L.e1(5);
}

contract C {
  event e1(uint indexed a);
  function g() public {
    f();
  }
}
