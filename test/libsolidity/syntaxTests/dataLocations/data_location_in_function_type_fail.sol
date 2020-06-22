library L {
    struct Nested { uint y; }
    function b(function(Nested calldata) external returns (uint)[] storage) external pure {}
    function d(function(Nested storage) external returns (uint)[] storage) external pure {}
}

// ----
// TypeError 6651: (159-173): Data location must be "memory" or "calldata" for parameter in function, but "storage" was given.
