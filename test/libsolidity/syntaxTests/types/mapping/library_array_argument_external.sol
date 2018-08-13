library L {
    function f(mapping(uint => uint)[] storage) external pure {
    }
}
// ----
// TypeError: (27-50): Type is required to live outside storage.
