function suffix(uint) pure suffix returns (function () external) {}

contract C {
    bytes4 x = 1000 suffix.selector;
}
// ----
// TypeError 9582: (102-117): Member "selector" not found or not visible after argument-dependent lookup in function (uint256) pure returns (function () external).
