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

using {addL as +, unsubL as -} for L.FixedBytes global;
using {addC as +, unsubC as -} for C.FixedBytes global;
using {addI as +, unsubI as -} for I.FixedBytes global;
// ----
// TypeError 4117: (545-600): Can only use "global" with types defined in the same source unit at file level.
// TypeError 4117: (601-656): Can only use "global" with types defined in the same source unit at file level.
// TypeError 4117: (657-712): Can only use "global" with types defined in the same source unit at file level.
