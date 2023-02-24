function s(uint) pure suffix returns (uint) {}
function s(uint) pure returns (uint) {}
// ----
// DeclarationError 1686: (0-46): Function with same name and parameter types defined twice.
