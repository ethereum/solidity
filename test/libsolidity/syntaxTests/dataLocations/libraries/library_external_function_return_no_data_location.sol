library L {
    struct S { uint x; }
    function g() external pure returns (uint[2]) {}
    function h() external pure returns (uint[]) {}
    function i() external pure returns (S) {}
    function j() external pure returns (mapping(uint => uint)) {}
}
// ----
// TypeError 6651: (77-84): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (129-135): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (180-181): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (226-247): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
