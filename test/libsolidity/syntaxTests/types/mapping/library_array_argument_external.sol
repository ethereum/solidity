library L {
    function f(mapping(uint => uint)[] storage) external pure {
    }
}
// ----
// TypeError 3312: (27-58): Type is required to live outside storage.
