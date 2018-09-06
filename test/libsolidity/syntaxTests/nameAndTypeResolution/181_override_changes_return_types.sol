contract base {
    function f(uint a) public returns (uint) { }
}
contract test is base {
    function f(uint a) public returns (uint8) { }
}
// ----
// TypeError: (95-140): Overriding function return types differ.
