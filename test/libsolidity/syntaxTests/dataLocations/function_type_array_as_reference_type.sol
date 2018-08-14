contract C {
    struct Nested { uint y; }
    // ensure that we consider array of function pointers as reference type
    function b(function(Nested memory) external returns (uint)[] storage) internal pure {}
    function c(function(Nested memory) external returns (uint)[] memory) public pure {}
    function d(function(Nested memory) external returns (uint)[] calldata) external pure {}
}
// ----
