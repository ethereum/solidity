contract C {
    function f() public view returns (bytes32) {
        return block.blockhash(3);
    }
}
// ----
// TypeError: (77-92): "block.blockhash()" has been deprecated in favor of "blockhash()"
