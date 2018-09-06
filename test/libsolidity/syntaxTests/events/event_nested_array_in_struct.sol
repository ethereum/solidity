contract c {
	struct S { uint x; uint[][] arr; }
    event E(S);
}
// ----
// TypeError: (61-62): This type is only supported in the new experimental ABI encoder. Use "pragma experimental ABIEncoderV2;" to enable the feature.
