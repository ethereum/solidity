contract c {
    bytes data;

    function foo() public returns (bytes32) {
        data.push("x");
        data.push("y");
        data.push("z");
        return keccak256(abi.encodePacked("b", keccak256(data), "a"));
    }
}
// ====
// compileViaYul: also
// ----
// foo() -> 0xb338eefce206f9f57b83aa738deecd5326dc4b72dd81ee6a7c621a6facb7acdc
