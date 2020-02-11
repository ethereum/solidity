contract c {
    bytes data;

    function foo() public returns(bytes32) {
        data.push("x");
        data.push("y");
        data.push("z");
        return keccak256(abi.encodePacked("b", keccak256(data), "a"));
    }
}

// ----
// foo() ->  util::keccak256(bytes{'b'} + util::keccak256("xyz".asBytes( + bytes{'a'} 
// foo():"" -> "81064592765372817159845741028275376000365320033790514016917613221788490640604"
