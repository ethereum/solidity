using {add as +} for S;
using {add as +} for Z;

function add(S memory, S memory) pure returns (S memory) {}
function add(Z memory, Z memory) pure returns (Z memory) {}

struct S { int x; }
struct Z { int x; }
// ----
// DeclarationError 7920: (7-10): Identifier not found or not unique.
