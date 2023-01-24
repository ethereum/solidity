using {L.add as +} for A;
using {L.add as +} for AP;

library L {
    function add(A, A) private pure returns (A) {}
    function add(AP, AP) internal pure returns (AP) {}
}

type A is address;
type AP is address payable;
// ----
// DeclarationError 7920: (7-12): Identifier not found or not unique.
