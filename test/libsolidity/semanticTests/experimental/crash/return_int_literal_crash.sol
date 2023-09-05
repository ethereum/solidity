pragma experimental solidity;

type X = word;

class T: TC {}

instantiation X: TC {
    function f() -> bool {
        return 1;
    }
}
// ----
// () ->
