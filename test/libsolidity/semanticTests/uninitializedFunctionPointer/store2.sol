pragma solidity ^0.5.7;

contract InvalidTest {

  function() internal storedFn;
  uint public x;

  constructor() public {
    uint _y1;
    uint _y2;
    uint _y3;
    uint _y4;
    uint _y5;
    uint _y6;
    uint _y7;
    uint _y8;
    uint _y9;
    uint _y10;
    uint _y11;
    uint _y12;
    uint _y13;
    uint _y14;


    function() internal invalid;
    storedFn = invalid;
  }

  function run() public {
    // this did not always cause revert in the past
    storedFn();
  }

  function z() public {
      x++;
  }
}
// ----
// run() -> FAILURE
