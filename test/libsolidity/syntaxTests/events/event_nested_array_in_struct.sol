contract c {
	struct S { uint x; uint[][] arr; }
    event E(S);
}
// ----
// TypeError 3061: (61-62): This type is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
