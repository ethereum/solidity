interface HelloWorld {
    function hello() external pure;
    function world(int) external pure;
}

interface HelloWorldDerived is HelloWorld {
    function other() external pure;
}

interface ERC165 {
    /// @notice Query if a contract implements an interface
    /// @param interfaceID The interface identifier, as specified in ERC-165
    /// @dev Interface identification is specified in ERC-165. This function
    ///  uses less than 30,000 gas.
    /// @return `true` if the contract implements `interfaceID` and
    ///  `interfaceID` is not 0xffffffff, `false` otherwise
    function supportsInterface(bytes4 interfaceID) external view returns (bool);
}

contract Test {
    bytes4 public ghello_world_interfaceId = type(HelloWorld).interfaceId;
    bytes4 public ERC165_interfaceId = type(ERC165).interfaceId;

    function hello() public pure returns (bytes4 data){
        HelloWorld i;
        return i.hello.selector;
    }

    function world() public pure returns (bytes4 data){
        HelloWorld i;
        return i.world.selector;
    }

    function hello_world() public pure returns (bytes4 data){
        // HelloWorld i;
        // return i.hello.selector ^ i.world.selector; // = 0xc6be8b58
        return 0xc6be8b58;
    }

    function hello_world_interfaceId() public pure returns (bytes4 data){
        return type(HelloWorld).interfaceId;
    }

    function other() public pure returns (bytes4 data){
        HelloWorldDerived i;
        return i.other.selector;
    }

    function hello_world_derived_interfaceId() public pure returns (bytes4 data){
        return type(HelloWorldDerived).interfaceId;
    }
}
// ----
// hello() -> left(0x19ff1d21)
// world() -> left(0xdf419679)
// ERC165_interfaceId() -> left(0x01ffc9a7)
// hello_world() -> left(0xc6be8b58)
// hello_world_interfaceId() -> left(0xc6be8b58)
// ghello_world_interfaceId() -> left(0xc6be8b58)
// other() -> left(0x85295877)
// hello_world_derived_interfaceId() -> left(0x85295877)
