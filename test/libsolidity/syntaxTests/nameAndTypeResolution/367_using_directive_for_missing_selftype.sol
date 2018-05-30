library B {
    function b() public {}
}

contract A {
    using B for bytes;

    function a() public {
        bytes memory x;
        x.b();
    }
}
// ----
// TypeError: (137-140): Member "b" not found or not visible after argument-dependent lookup in bytes memory
