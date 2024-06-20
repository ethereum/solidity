contract C {
  function f() public pure {
    fixed f1 = 3.14_15;
    fixed f2 = 3_1.4_15;

    f1; f2;
  }
}
// ----
// UnimplementedFeatureError 1834: Not yet implemented - FixedPointType.
