library L {
    type FixedBytes is bytes1;
}

contract C {
    type FixedBytes is bytes2;
}

interface I {
    type FixedBytes is bytes3;
}

function addL(L.FixedBytes, L.FixedBytes) pure returns (L.FixedBytes) {}
function addC(C.FixedBytes, C.FixedBytes) pure returns (C.FixedBytes) {}
function addI(I.FixedBytes, I.FixedBytes) pure returns (I.FixedBytes) {}

function unsubL(L.FixedBytes) pure returns (L.FixedBytes) {}
function unsubC(C.FixedBytes) pure returns (C.FixedBytes) {}
function unsubI(I.FixedBytes) pure returns (I.FixedBytes) {}

using {addL as +, unsubL as -} for L.FixedBytes;
using {addC as +, unsubC as -} for C.FixedBytes;
using {addI as +, unsubI as -} for I.FixedBytes;
// ----
// TypeError 3320: (552-556): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (563-569): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (601-605): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (612-618): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (650-654): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (661-667): Operators can only be defined in a global 'using for' directive.
