pragma experimental ABIEncoderV2;

contract C {
  uint[][2] tmp_i;
  function i(uint[][2] calldata s) external { tmp_i = s; }
}

// ----
// UnimplementedFeatureError: Copying nested calldata dynamic arrays to storage is not implemented in the old code generator.
