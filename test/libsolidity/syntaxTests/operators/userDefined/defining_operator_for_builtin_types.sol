using {f as +} for uint global;
using {f as +} for uint[2] global;
using {f as +} for mapping(uint => uint) global;
using {f as +} for function (uint) pure returns (uint) global;
using {f as +} for string global;

function f(uint, uint) pure returns (uint) {}
// ----
// TypeError 8841: (0-31): Can only use "global" with user-defined types.
// TypeError 5332: (7-8): Operators can only be implemented for user-defined value types.
// TypeError 8841: (32-66): Can only use "global" with user-defined types.
// TypeError 5332: (39-40): Operators can only be implemented for user-defined value types.
// TypeError 8841: (67-115): Can only use "global" with user-defined types.
// TypeError 5332: (74-75): Operators can only be implemented for user-defined value types.
// TypeError 8841: (116-178): Can only use "global" with user-defined types.
// TypeError 5332: (123-124): Operators can only be implemented for user-defined value types.
// TypeError 8841: (179-212): Can only use "global" with user-defined types.
// TypeError 5332: (186-187): Operators can only be implemented for user-defined value types.
