contract c {
    function f1(mapping(uint => uint)[] calldata) pure external {}
}
// ----
// TypeError: (29-61): Types containing (nested) mappings can only be used in storage.
// TypeError: (29-61): Only libraries are allowed to use a (nested) mapping type in public or external functions.
