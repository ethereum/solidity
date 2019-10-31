contract base {
    function f(uint a) public returns (uint) { }
}
contract test is base {
    function f(uint a) public override returns (uint8) { }
}
// ----
// TypeError: (95-149): Overriding function return types differ.
