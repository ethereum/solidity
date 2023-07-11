contract C { function decrement(uint x) private pure returns (uint) { return x - 1; } function decrement2(uint x) private pure returns (uint) { return x - 1; } function f(uint x) public pure returns (uint) { return x > 0 ? decrement(x) : decrement2(x); } }
// ----
// Warning 3944: (151-156): CHC: Underflow (resulting value less than 0) happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
