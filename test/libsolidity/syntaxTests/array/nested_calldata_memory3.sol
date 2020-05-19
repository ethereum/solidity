pragma experimental ABIEncoderV2;

contract Test {
    struct shouldBug {
        uint256[][] deadly;
    }
    function killer(uint256[][] calldata weapon) pure external {
      shouldBug(weapon);
    }
}

// ----
// UnimplementedFeatureError: Copying nested dynamic calldata arrays to memory is not implemented in the old code generator.
