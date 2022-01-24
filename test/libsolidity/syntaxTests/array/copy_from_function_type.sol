// Example from https://github.com/ethereum/solidity/issues/12558
pragma abicoder v2;
contract C {
  function() external[1][] s0;
  constructor(function() external[1][] memory i0)
  {
    i0[0] = s0[1];
  }
}
