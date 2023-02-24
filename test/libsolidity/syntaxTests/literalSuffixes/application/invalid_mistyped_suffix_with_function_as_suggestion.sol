function suffix(uint) pure returns (uint) {}

contract C {
    uint x = 1 suffx;
}
// ----
// DeclarationError 7576: (74-79): Undeclared identifier. Did you mean "suffix"?
