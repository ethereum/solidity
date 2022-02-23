pragma abicoder               v2;

contract Test {
    struct shouldBug {
        uint256[][] deadly;
    }
    function killer(uint256[][] calldata weapon) pure external {
      shouldBug(weapon);
    }
}

// ----
