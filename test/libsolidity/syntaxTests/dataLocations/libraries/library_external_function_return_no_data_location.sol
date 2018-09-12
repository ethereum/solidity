library L {
    struct S { uint x; }
    function g() external pure returns (uint[2]) {}
    function h() external pure returns (uint[]) {}
    function i() external pure returns (S) {}
    function j() external pure returns (mapping(uint => uint)) {}
}
// ----
// TypeError: (77-84): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (129-135): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (180-181): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (226-247): Data location must be "storage" or "memory" for return parameter in function, but none was given.
