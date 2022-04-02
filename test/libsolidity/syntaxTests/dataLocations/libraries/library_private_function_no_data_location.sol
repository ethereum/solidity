library L {
    struct S { uint x; }
    function g() private pure returns (uint[2]) {}
    function h() private pure returns (uint[]) {}
    function i() private pure returns (S) {}
    function j() private pure returns (mapping(uint => uint)) {}
    function gp(uint[2]) private pure {}
    function hp(uint[]) private pure {}
    function ip(S) private pure {}
    function jp(mapping(uint => uint)) private pure {}
}
// ----
// TypeError 6651: (76-83='uint[2]'): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (127-133='uint[]'): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (177-178='S'): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (222-243='mapping(uint => uint)'): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
// TypeError 6651: (264-271='uint[2]'): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
// TypeError 6651: (305-311='uint[]'): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
// TypeError 6651: (345-346='S'): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
// TypeError 6651: (380-401='mapping(uint => uint)'): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
