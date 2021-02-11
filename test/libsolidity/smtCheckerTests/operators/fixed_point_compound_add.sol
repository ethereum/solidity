pragma experimental SMTChecker;
contract C {
  fixed[] b;
  function f() internal { b[0] += 1; }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
