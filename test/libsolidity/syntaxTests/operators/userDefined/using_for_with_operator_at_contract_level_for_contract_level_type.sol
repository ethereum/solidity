library L {
    type FixedBytes is bytes10;
}

function add(L.FixedBytes, L.FixedBytes) pure returns (L.FixedBytes) {}
function unsub(L.FixedBytes, L.FixedBytes) pure returns (L.FixedBytes) {}

library LX {
    using {add as +, unsub as -} for L.FixedBytes;
}

contract CX {
    using {add as +, unsub as -} for L.FixedBytes;
}
// ----
// TypeError 3320: (218-221): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (228-233): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (286-289): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (296-301): Operators can only be defined in a global 'using for' directive.
