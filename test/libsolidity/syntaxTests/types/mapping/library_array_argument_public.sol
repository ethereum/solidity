library L {
    function f(mapping(uint => uint)[] storage) public pure {
    }
}
// ----
// TypeError 3312: (27-58): Type is required to live outside storage.
