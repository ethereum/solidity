library L {
    struct Nested { uint y; }
    function c(function(Nested memory) external returns (uint)[] storage) external pure {}
}
