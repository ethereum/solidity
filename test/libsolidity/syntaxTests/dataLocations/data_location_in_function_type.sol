library L {
    struct Nested { uint y; }
    // data location specifier in function signature should be optional even if there are multiple choices
    function a(function(Nested) external returns (uint)[] storage) external pure {}
    function b(function(Nested calldata) external returns (uint)[] storage) external pure {}
    function c(function(Nested memory) external returns (uint)[] storage) external pure {}
    function d(function(Nested storage) external returns (uint)[] storage) external pure {}
}

// ----
// TypeError: (441-447): Location has to be calldata or memory for function type of external function. Remove the data location keyword to fix this error.
