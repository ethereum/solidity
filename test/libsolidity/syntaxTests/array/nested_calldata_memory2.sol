pragma abicoder               v2;

contract Test {
    struct shouldBug {
        uint256[][2] deadly;
    }
    function killer(uint256[][2] calldata weapon) pure external {
      shouldBug(weapon);
    }
}

// ----
