contract A {
    uint dummy;
    function f(uint[] calldata) external pure {}
    function g(uint[] calldata) external view { dummy; }
    function h(uint[] calldata) external { dummy = 42; }
    function i(uint[] calldata) external payable {}
}
contract B is A {
    function f(uint[] memory) public override pure {}
    function g(uint[] memory) public override view { dummy; }
    function h(uint[] memory) public override { dummy = 42; }
    function i(uint[] memory) public override payable {}
}
