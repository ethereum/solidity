contract C {
    function literalToUFixed() public pure {
        ufixed8x2 a = 0.10;
        ufixed8x2 b = 0.00;
        ufixed8x2 c = 2.55;
        a; b; c;
    }
    function literalToFixed() public pure {
        fixed8x1 a =   0.1;
        fixed8x1 b =  12.7;
        fixed8x1 c = -12.8;
        a; b; c;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
