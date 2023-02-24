library L {
    function f(uint) internal pure suffix returns (uint) {}
}
// ----
// DeclarationError 7920: (47-53): Identifier not found or not unique.
