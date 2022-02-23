function f() pure returns (uint) { return 1337; }
function f() pure returns (uint) { return 42; }
function f() pure returns (uint) { return 1; }
// ----
// DeclarationError 1686: (0-49): Function with same name and parameter types defined twice.
