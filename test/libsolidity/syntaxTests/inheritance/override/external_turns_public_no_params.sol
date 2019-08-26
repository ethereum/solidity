contract A {
    function f() external pure {}
}
contract B is A {
    function f() public override pure {
    }
}
