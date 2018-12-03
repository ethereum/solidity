interface I {
    function f(uint[] calldata) external pure;
    function g(uint[] calldata) external view;
    function h(uint[] calldata) external;
    function i(uint[] calldata) external payable;
}
contract C is I {
    uint dummy;
    function f(uint[] memory) public pure {}
    function g(uint[] memory) public view { dummy; }
    function h(uint[] memory) public { dummy = 42; }
    function i(uint[] memory) public payable {}
}
