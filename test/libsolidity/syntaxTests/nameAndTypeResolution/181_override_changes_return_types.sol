contract base {
    function f(uint a) public virtual returns (uint) { }
}
contract test is base {
    function f(uint a) public override returns (uint8) { }
}
// ----
// TypeError 4822: (103-157): Overriding function return types differ.
