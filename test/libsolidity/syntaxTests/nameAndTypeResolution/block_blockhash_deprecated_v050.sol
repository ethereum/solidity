pragma experimental "v0.5.0";
contract C {
    function f() public returns (bytes32) { return block.blockhash(3); }
}
// ----
// TypeError: (94-109): "block.blockhash()" has been deprecated in favor of "blockhash()"
