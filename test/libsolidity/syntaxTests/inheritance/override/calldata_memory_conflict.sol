contract A {
    uint dummy;
    function f(uint[] calldata) external pure {}
    function g(uint[] calldata) external view { dummy; }
    function h(uint[] calldata) external { dummy = 42; }
    function i(uint[] calldata) external payable {}
}
contract B is A {
    function f(uint[] calldata) external pure {}
    function g(uint[] calldata) external view { dummy; }
    function h(uint[] calldata) external { dummy = 42; }
    function i(uint[] calldata) external payable {}
    function f(uint[] memory) public pure {}
    function g(uint[] memory) public view { dummy; }
    function h(uint[] memory) public { dummy = 42; }
    function i(uint[] memory) public payable {}
}
// ----
// DeclarationError: (268-312): Function with same name and arguments defined twice.
// DeclarationError: (317-369): Function with same name and arguments defined twice.
// DeclarationError: (374-426): Function with same name and arguments defined twice.
// DeclarationError: (431-478): Function with same name and arguments defined twice.
// TypeError: (268-312): Overriding function visibility differs.
// TypeError: (317-369): Overriding function visibility differs.
// TypeError: (374-426): Overriding function visibility differs.
// TypeError: (431-478): Overriding function visibility differs.
