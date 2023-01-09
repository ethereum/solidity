contract A {
    uint public data;
}

contract C {
    A a = new A();

    using {a.data} for uint;
}
// ----
// DeclarationError 9589: (82-88): Identifier is not a function name or not unique.
