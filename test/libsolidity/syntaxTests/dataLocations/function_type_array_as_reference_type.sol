contract C {
    struct Nested { uint y; }
    // ensure that we consider array of function pointers as reference type
    function a(function(Nested) external returns (uint)[]) public pure {}
    function b(function(Nested) external returns (uint)[] storage) public pure {}
    function c(function(Nested) external returns (uint)[] memory) public pure {}
    function d(function(Nested) external returns (uint)[] calldata) public pure {}
}
// ----
// TypeError: (208-250): Location has to be memory for public function. Remove the data location keyword to fix this error.
