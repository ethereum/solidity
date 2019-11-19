contract A {
    function f() external virtual pure {}
}
contract B is A {
    function f() public override pure {
    }
}
