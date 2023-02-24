interface I {
    function f(uint) external pure suffix returns (uint) {}
}
// ----
// SyntaxError 5842: (18-73): Functions in interfaces cannot have modifiers.
// DeclarationError 7920: (49-55): Identifier not found or not unique.
