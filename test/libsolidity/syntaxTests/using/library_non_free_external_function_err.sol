library L {
    function id_ext(uint x) external returns(uint) {
        return x;
    }
}

contract C {
    using L.id_ext for uint;
    function f(uint x) external {
        x.id_ext();
    }
}
// ----
// DeclarationError 7920: (115-123): Identifier not found or not unique.
