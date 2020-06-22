contract c {
    struct S { uint a ; }
    event E(S);
}
// ----
// TypeError 3061: (51-52): This type is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
