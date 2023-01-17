using {add as +, sub as -, mul as *} for A;

function add(A, A) view returns (A) {}
function sub(A, A) returns (A) {}
function mul(A, A) payable returns (A) {}

type A is address payable;
// ----
// TypeError 7775: (7-10): Only pure functions can be used to define operators.
// TypeError 7775: (17-20): Only pure functions can be used to define operators.
// TypeError 7775: (27-30): Only pure functions can be used to define operators.
// TypeError 9559: (118-159): Free functions cannot be payable.
