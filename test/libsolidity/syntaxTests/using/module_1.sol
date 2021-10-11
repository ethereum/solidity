==== Source: A ====
function id(uint x) pure returns (uint) {
    return x;
}

==== Source: B ====
import "A" as M;
contract C {
    using M for uint;
}
// ----
// TypeError 4357: (B:40-41): Library name expected. If you want to attach a function, use '{...}'.
