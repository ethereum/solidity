contract test {
    function f() public pure {
        fixed a = 0.42578125 % -0.4271087646484375;
        fixed b = .5 % a;
        fixed c = a % b;
        a; b; c;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
