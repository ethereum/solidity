contract c {
    function f1(mapping(uint => uint)[] calldata) pure external {}
}
// ----
// TypeError 3312: (29-61): Type is required to live outside storage.
// TypeError 4103: (29-61): Only libraries are allowed to use the mapping type in public or external functions.
