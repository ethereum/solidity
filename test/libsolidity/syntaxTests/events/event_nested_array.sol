contract c {
    event E(uint[][]);
}
// ----
// TypeError: (25-33): This type is only supported in the new experimental ABI encoder. Use "pragma experimental ABIEncoderV2;" to enable the feature.
