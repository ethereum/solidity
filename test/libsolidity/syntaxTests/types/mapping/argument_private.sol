// This is expected to fail now, but may work in the future.
contract C {
    function f(mapping(uint => uint) storage) private pure {
    }
}
// ----
// TypeError: (89-110): Type is required to live outside storage.
