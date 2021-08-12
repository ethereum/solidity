contract C {
    function f() public view returns (uint) {
        return block.basefee;
    }
}
// ====
// EVMVersion: <=berlin
// ----
// TypeError 5921: (74-87): "basefee" is not supported by the VM version.
