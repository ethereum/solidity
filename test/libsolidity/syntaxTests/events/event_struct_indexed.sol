contract c {
    struct S { uint a ; }
    event E(S indexed);
}
// ----
// TypeError 3061: (51-60): This type is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
