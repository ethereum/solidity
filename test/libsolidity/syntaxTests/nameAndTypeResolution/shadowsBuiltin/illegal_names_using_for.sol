library L {
    function f() public {
    }
}

// error
struct super {
    uint a;
}

// error
struct this {
    uint a;
}

// error
struct _ {
    uint a;
}

contract C {
    // These are not errors
    using L for super;
    using L for _;
    using L for this;
}
// ----
// DeclarationError 3726: (56-84): The name "super" is reserved.
// DeclarationError 3726: (95-122): The name "this" is reserved.
// DeclarationError 3726: (133-157): The name "_" is reserved.
// Warning 2319: (56-84): This declaration shadows a builtin symbol.
// Warning 2319: (95-122): This declaration shadows a builtin symbol.
