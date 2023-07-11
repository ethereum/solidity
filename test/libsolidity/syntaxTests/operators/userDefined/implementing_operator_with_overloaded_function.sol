using {add as +} for A global;
using {add as +} for AP global;

function add(A, A) pure returns (A) {}
function add(AP, AP) pure returns (AP) {}

type A is address;
type AP is address payable;
// ----
// DeclarationError 9589: (7-10): Identifier is not a function name or not unique.
