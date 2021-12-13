pragma abicoder               v2;

contract test {
    struct S {
        T t;
    }
    struct T {
        mapping (uint => uint) k;
    }
    function f(S calldata b) external {
    }
}
// ----
// TypeError 4103: (155-167): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
