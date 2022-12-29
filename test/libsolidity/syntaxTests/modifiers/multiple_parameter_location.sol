contract A {
    modifier mod1(string storage storage a) { _; }
    modifier mod2(string storage memory a) { _; }
    modifier mod3(string storage calldata a) { _; }
    modifier mod4(string memory storage a) { _; }
    modifier mod5(string memory memory a) { _; }
    modifier mod6(string memory calldata a) { _; }
    modifier mod7(string calldata storage a) { _; }
    modifier mod8(string calldata memory a) { _; }
    modifier mod9(string calldata calldata a) { _; }
}
// ----
// ParserError 3548: (46-53): Location already specified.
// ParserError 3548: (97-103): Location already specified.
// ParserError 3548: (147-155): Location already specified.
// ParserError 3548: (198-205): Location already specified.
// ParserError 3548: (248-254): Location already specified.
// ParserError 3548: (297-305): Location already specified.
// ParserError 3548: (350-357): Location already specified.
// ParserError 3548: (402-408): Location already specified.
// ParserError 3548: (453-461): Location already specified.
