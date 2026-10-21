abstract contract C {
    function(uint x1) ptr1; // Shadows the state variable despite the order
    uint x1;

    function g(uint x2) public virtual;
    uint x2; // No shadowing (no body)

    function f(uint x4) public {
        function(uint x3) ptr3;
        uint x3; // No shadowing (due to order)

        (x1, ptr1) = (x3, ptr3);
    }
    uint x4; // Shadows f's parameter
}

abstract contract D {
    uint y1;
    function(uint y1) ptr1; // Shadows the state variable

    uint y2;
    function g(uint y2) public virtual; // No shadowing (no body)

    uint y4;
    function f(uint y4) public { // Shadows the state variable
        uint y3;
        function(uint y3) ptr3; // Shadows the local variable

        (y1, ptr1) = (y3, ptr3);
    }
}
// ----
// Warning 6162: (35-42): Naming function type parameters is deprecated.
// Warning 6162: (242-249): Naming function type parameters is deprecated.
// Warning 6162: (434-441): Naming function type parameters is deprecated.
// Warning 6162: (670-677): Naming function type parameters is deprecated.
// Warning 2519: (35-42): This declaration shadows an existing declaration.
// Warning 2519: (207-214): This declaration shadows an existing declaration.
// Warning 2519: (434-441): This declaration shadows an existing declaration.
// Warning 2519: (670-677): This declaration shadows an existing declaration.
// Warning 2519: (588-595): This declaration shadows an existing declaration.
// Warning 5667: (207-214): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (242-249): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (588-595): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (670-677): Unused function parameter. Remove or comment out the variable name to silence this warning.
