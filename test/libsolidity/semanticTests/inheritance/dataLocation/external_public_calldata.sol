abstract contract A {
    function f(uint256[] calldata a) external virtual returns (uint256[] calldata);
}

contract B is A {
    function f(uint256[] memory a) public override returns (uint256[] memory) {
        return a;
    }

    function g(uint[] calldata x) public returns (uint256[] memory) {
        return f(x);
    }
}
// ====
// compileViaYul: also
// ----
// f(uint256[]): 0x20, 2, 9, 8 -> 0x20, 2, 9, 8
// g(uint256[]): 0x20, 2, 9, 8 -> 0x20, 2, 9, 8
