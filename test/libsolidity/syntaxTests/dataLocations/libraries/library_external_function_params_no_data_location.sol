library L {
    struct S { uint x; }
    function g(uint[2]) external pure {}
    function h(uint[]) external pure {}
    function i(S) external pure {}
    function j(mapping(uint => uint)) external pure {}
}
// ----
// TypeError 6651: (52-59='uint[2]'): Data location must be "storage", "memory" or "calldata" for parameter in external function, but none was given.
// TypeError 6651: (93-99='uint[]'): Data location must be "storage", "memory" or "calldata" for parameter in external function, but none was given.
// TypeError 6651: (133-134='S'): Data location must be "storage", "memory" or "calldata" for parameter in external function, but none was given.
// TypeError 6651: (168-189='mapping(uint => uint)'): Data location must be "storage", "memory" or "calldata" for parameter in external function, but none was given.
