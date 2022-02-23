// Used to trigger assert
contract s{}
function f() {s[:][];}
// ----
// TypeError 1760: (53-57): Types cannot be sliced.
