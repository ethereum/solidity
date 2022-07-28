using {add as +, sub as -, mul as *} for A global;

function add(A, A) view returns (A) {}
function sub(A, A) returns (A) {}
function mul(A, A) payable returns (A) {}

type A is address payable;
// ----
// TypeError 7775: (7-10): Only pure free functions can be used to define operators.
// TypeError 7775: (17-20): Only pure free functions can be used to define operators.
// TypeError 7775: (27-30): Only pure free functions can be used to define operators.
// TypeError 9559: (125-166): Free functions cannot be payable.
