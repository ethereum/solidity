library L
{
    function f(mapping(uint => uint) storage a, mapping(uint => uint) storage b, bool c) external pure returns(mapping(uint => uint) storage) {
        return c ? a : b;
    }
}
// ----
// TypeError: (27-58): Type is required to live outside storage.
// TypeError: (60-91): Type is required to live outside storage.
// TypeError: (123-152): Type is required to live outside storage.
