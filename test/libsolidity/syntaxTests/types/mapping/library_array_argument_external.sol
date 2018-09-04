library L {
    function f(mapping(uint => uint)[] storage) external pure {
    }
}
// ----
// TypeError: (27-58): Type is required to live outside storage.
