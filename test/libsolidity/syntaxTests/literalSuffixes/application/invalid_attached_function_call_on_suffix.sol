function suffix(uint) pure suffix returns (uint) {}

library L {
    function add(uint, uint) internal pure returns (uint) {}
}

contract C {
    using L for *;

    uint x = 1000 suffix.add(1);
}
// ----
// TypeError 9582: (180-190): Member "add" not found or not visible after argument-dependent lookup in function (uint256) pure returns (uint256).
