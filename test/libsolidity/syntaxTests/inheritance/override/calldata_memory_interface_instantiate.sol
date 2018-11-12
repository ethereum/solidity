interface I {
    function f(uint[] calldata) external pure;
}
contract A is I {
    function f(uint[] memory) public pure {}
}
contract C {
    function f() public {
        I i = I(new A());
        i.f(new uint[](1));
    }
}