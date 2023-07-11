using {L.add as +} for A global;
using {L.add as +} for AP global;

library L {
    function add(A, A) private pure returns (A) {}
    function add(AP, AP) internal pure returns (AP) {}
}

type A is address;
type AP is address payable;
// ----
// DeclarationError 9589: (7-12): Identifier is not a function name or not unique.
