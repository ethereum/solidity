library L {
    struct Nested { uint y; }
    function b(function(Nested calldata) external returns (uint)[] storage) external pure {}
    function d(function(Nested storage) external returns (uint)[] storage) external pure {}
    function f(function(Nested transient) external returns (uint)[] storage) external pure {}
}

// ----
// Warning 6162: (251-267): Naming function type parameters is deprecated.
// TypeError 6651: (159-173): Data location must be "memory" or "calldata" for parameter in function, but "storage" was given.
// TypeError 6651: (251-267): Data location must be "memory" or "calldata" for parameter in function, but none was given.
