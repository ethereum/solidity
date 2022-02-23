enum this {
    a
}
enum super {
    b
}
enum _ {
    c
}

enum E {
    this,
    super,
    _
}

contract C {
    this a;
    super b;
    _ c;
    E e;
}
// ----
// DeclarationError 3726: (0-19): The name "this" is reserved.
// DeclarationError 3726: (20-40): The name "super" is reserved.
// DeclarationError 3726: (41-57): The name "_" is reserved.
// DeclarationError 3726: (72-76): The name "this" is reserved.
// DeclarationError 3726: (82-87): The name "super" is reserved.
// DeclarationError 3726: (93-94): The name "_" is reserved.
// Warning 2319: (0-19): This declaration shadows a builtin symbol.
// Warning 2319: (20-40): This declaration shadows a builtin symbol.
