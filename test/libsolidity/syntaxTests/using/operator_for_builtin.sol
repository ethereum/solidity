using {f as +} for uint;
function f(uint, uint) pure returns (uint) {}
// ----
// TypeError 5332: (7-8): Operators can only be implemented for user-defined types and not for contracts.
