contract C {
    fixed a1 = 0.1 % -0.4271087646484375;
    fixed a2 = 0.1 % 0.4271087646484375;
    fixed a3 = 0 / 0.123;
    fixed a4 = 0 / -0.123;
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
