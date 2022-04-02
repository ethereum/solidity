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
// TypeError 6651: (77-84='uint[2]'): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (129-135='uint[]'): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (180-181='S'): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (226-247='mapping(uint => uint)'): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (268-275='uint[2]'): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
// TypeError 6651: (310-316='uint[]'): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
// TypeError 6651: (351-352='S'): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
// TypeError 6651: (387-408='mapping(uint => uint)'): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
