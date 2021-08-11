contract C {
    function f() public view returns (uint) {
        return block.basefee;
    }
    function g() public view returns (uint ret) {
        assembly {
            ret := basefee()
        }
    }
}
// ====
// EVMVersion: =berlin
// ----
// TypeError 5921: (74-87): "basefee" is not supported by the VM version.
// TypeError 5430: (183-190): The "basefee" instruction is only available for London-compatible VMs (you are currently compiling for "berlin").
