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
// TypeError: (76-83): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (127-133): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (177-178): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (222-243): Data location must be "storage" or "memory" for return parameter in function, but none was given.
// TypeError: (264-271): Data location must be "storage" or "memory" for parameter in function, but none was given.
// TypeError: (305-311): Data location must be "storage" or "memory" for parameter in function, but none was given.
// TypeError: (345-346): Data location must be "storage" or "memory" for parameter in function, but none was given.
// TypeError: (380-401): Data location must be "storage" or "memory" for parameter in function, but none was given.
