==== Source: A ====
function id(uint x) pure returns (uint) {
    return x;
}

==== Source: B ====
import "A" as M;

contract C {
    using { id } for uint;
}
// ----
// DeclarationError 7920: (B:43-45): Identifier not found or not unique.
