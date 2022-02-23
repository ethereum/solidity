pragma abicoder               v2;

contract Test {
    struct MyStructName {
        address addr;
        MyStructName[] x;
    }

    function f(MyStructName memory s) public {}
}
// ----
// TypeError 4103: (147-168): Recursive type not allowed for public or external contract functions.
