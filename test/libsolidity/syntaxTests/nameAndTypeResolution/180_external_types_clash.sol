contract base {
    enum a { X }
    function f(a) public { }
}
contract test is base {
    function f(uint8 a) public { }
}
// ----
// TypeError: (37-61): Function overload clash during conversion to external types for arguments.
