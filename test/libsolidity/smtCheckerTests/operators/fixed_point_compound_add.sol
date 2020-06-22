pragma experimental SMTChecker;
contract C {
  fixed[] b;
  function f() internal { b[0] += 1; }
}
// ----
// Warning 4144: (84-93): Underflow (resulting value less than 0) happens here
// Warning 2661: (84-93): Overflow (resulting value larger than 2**256 - 1) happens here
