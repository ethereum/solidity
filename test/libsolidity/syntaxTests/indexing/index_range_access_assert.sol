// Used to trigger assert
contract s{}
function f() {s[:][];}
// ----
// TypeError 1760: (53-57): Types cannot be sliced.
// TypeError 2876: (53-59): Index access for contracts or libraries is not possible.
