error E();

contract C {
    function f() public pure {
        E x;
    }
}
// ----
// TypeError 5172: (64-65): Name has to refer to a struct, enum or contract.
