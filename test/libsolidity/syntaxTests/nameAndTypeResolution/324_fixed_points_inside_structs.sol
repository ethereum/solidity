contract test {
    struct myStruct {
        ufixed a;
        int b;
    }
    myStruct a = myStruct(3.125, 3);
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
