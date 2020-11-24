pragma abicoder               v2;

contract test {
    struct S {
        mapping (uint => uint) s;
    }
    function f(S calldata b) external {
    }
}
// ----
// TypeError 4103: (121-133): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
// TypeError 4061: (121-133): Type struct test.S is only valid in storage because it contains a (nested) mapping.
