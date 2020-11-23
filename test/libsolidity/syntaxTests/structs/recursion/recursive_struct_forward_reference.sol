pragma abicoder               v2;

contract C {
    function f(Data.S memory a) public {}
}
contract Data {
    struct S { S[] x; }
}
// ----
// TypeError 4103: (63-78): Recursive type not allowed for public or external contract functions.
