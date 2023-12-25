contract C {
    function f() public view returns (uint) {
        return block.blobbasefee;
    }
    function g() public view returns (uint ret) {
        assembly {
            ret := blobbasefee()
        }
    }
}
// ====
// EVMVersion: =shanghai
// ----
// TypeError 1006: (74-91): "blobbasefee" is not supported by the VM version.
// DeclarationError 4619: (187-198): Function "blobbasefee" not found.
// DeclarationError 8678: (180-200): Variable count for assignment to "ret" does not match number of values (1 vs. 0)
