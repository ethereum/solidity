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
// DeclarationError 3726: (56-84='struct super {     uint a; }'): The name "super" is reserved.
// DeclarationError 3726: (95-122='struct this {     uint a; }'): The name "this" is reserved.
// DeclarationError 3726: (133-157='struct _ {     uint a; }'): The name "_" is reserved.
// Warning 2319: (56-84='struct super {     uint a; }'): This declaration shadows a builtin symbol.
// Warning 2319: (95-122='struct this {     uint a; }'): This declaration shadows a builtin symbol.
