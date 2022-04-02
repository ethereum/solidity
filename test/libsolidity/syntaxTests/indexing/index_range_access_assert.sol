// Used to trigger assert
contract s{}
function f() {s[:][];}
// ----
// TypeError 1760: (53-57='s[:]'): Types cannot be sliced.
