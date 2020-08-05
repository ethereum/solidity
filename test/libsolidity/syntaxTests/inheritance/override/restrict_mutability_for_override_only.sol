contract A {
    // no "state mutability can be restricted"-warning here
    function foo() external virtual returns (uint) { return 1; }
}
contract B is A {
    // no "state mutability can be restricted"-warning here
    function foo() external virtual override returns (uint) { return 2; }
}
contract C is B {
    // warning is here
    function foo() external override returns (uint) { return 3; }
}
// ----
// Warning 2018: (339-400): Function state mutability can be restricted to pure
