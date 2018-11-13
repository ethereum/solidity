library L {
    struct S { uint x; }
    function g(uint[2]) external pure {}
    function h(uint[]) external pure {}
    function i(S) external pure {}
    function j(mapping(uint => uint)) external pure {}
}
// ----
// TypeError: (52-59): Data location must be "storage" or "calldata" for parameter in external function, but none was given.
// TypeError: (93-99): Data location must be "storage" or "calldata" for parameter in external function, but none was given.
// TypeError: (133-134): Data location must be "storage" or "calldata" for parameter in external function, but none was given.
// TypeError: (168-189): Data location must be "storage" or "calldata" for parameter in external function, but none was given.
