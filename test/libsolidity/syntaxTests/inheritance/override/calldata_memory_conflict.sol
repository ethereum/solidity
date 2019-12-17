contract A {
    uint dummy;
    function f(uint[] calldata) external virtual pure {}
    function g(uint[] calldata) external virtual view { dummy; }
    function h(uint[] calldata) external virtual { dummy = 42; }
    function i(uint[] calldata) external virtual payable {}
}
contract B is A {
    function f(uint[] calldata) external override pure {}
    function g(uint[] calldata) external override view { dummy; }
    function h(uint[] calldata) external override { dummy = 42; }
    function i(uint[] calldata) external override payable {}
    function f(uint[] memory) public override pure {}
    function g(uint[] memory) public override view { dummy; }
    function h(uint[] memory) public override { dummy = 42; }
    function i(uint[] memory) public override payable {}
}
// ----
// DeclarationError: (300-353): Function with same name and arguments defined twice.
// DeclarationError: (358-419): Function with same name and arguments defined twice.
// DeclarationError: (424-485): Function with same name and arguments defined twice.
// DeclarationError: (490-546): Function with same name and arguments defined twice.
