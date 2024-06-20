library L {
    function f1(uint[] transient) private pure { }
    function f2() private pure returns (uint[] transient) { }
    function g1(uint[] transient) internal pure { }
    function g2() internal pure returns (uint[] transient) { }
    function h1(uint[] transient) public pure { }
    function h2() public pure returns (uint[] transient) { }
    function i1(uint[] transient) external pure { }
    function i2() external pure returns (uint[] transient) { }
}
// ----
// TypeError 6651: (28-44): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
// TypeError 6651: (103-119): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (141-157): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
// TypeError 6651: (218-234): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (256-272): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
// TypeError 6651: (329-345): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (367-383): Data location must be "storage", "memory" or "calldata" for parameter in external function, but none was given.
// TypeError 6651: (444-460): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
