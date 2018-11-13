library L {
    struct S { uint x; }
    function g() internal pure returns (uint[2]) {}
    function h() internal pure returns (uint[]) {}
    function i() internal pure returns (S) {}
    function j() internal pure returns (mapping(uint => uint)) {}
    function gp(uint[2]) internal pure {}
    function hp(uint[]) internal pure {}
    function ip(S) internal pure {}
    function jp(mapping(uint => uint)) internal pure {}
}
// ----
// TypeError: (77-84): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (129-135): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (180-181): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (226-247): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (268-275): Data location must be "storage" or "memory" for parameter in function, but none was given.
// TypeError: (310-316): Data location must be "storage" or "memory" for parameter in function, but none was given.
// TypeError: (351-352): Data location must be "storage" or "memory" for parameter in function, but none was given.
// TypeError: (387-408): Data location must be "storage" or "memory" for parameter in function, but none was given.
