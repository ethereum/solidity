contract C {
    function f(uint) internal pure suffix returns (uint) {}
}
// ----
// DeclarationError 7920: (48-54): Identifier not found or not unique.
