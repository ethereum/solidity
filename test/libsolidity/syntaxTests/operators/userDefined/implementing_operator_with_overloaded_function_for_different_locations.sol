using {add as +} for S;

function add(S memory, S memory) pure returns (S memory) {}
function add(S storage, S storage) pure returns (S storage) {}

struct S { int x; }
// ----
// DeclarationError 7920: (7-10): Identifier not found or not unique.
