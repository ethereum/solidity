function exp(uint base, uint exponent) pure returns (uint power) {
    if (exponent == 0)
        return 1;
    power = exp(base, exponent / 2);
    power *= power;
    if (exponent & 1 == 1)
        power *= base;
}

contract C {
  function g(uint base, uint exponent) public pure returns (uint) {
      return exp(base, exponent);
  }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// g(uint256,uint256): 0, 0 -> 1
// g(uint256,uint256): 0, 1 -> 0x00
// g(uint256,uint256): 1, 0 -> 1
// g(uint256,uint256): 2, 3 -> 8
// g(uint256,uint256): 3, 10 -> 59049
// g(uint256,uint256): 2, 255 -> -57896044618658097711785492504343953926634992332820282019728792003956564819968
