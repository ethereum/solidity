interface ERC165 {
    /// @notice Query if a contract implements an interface
    /// @param interfaceID The interface identifier, as specified in ERC-165
    /// @dev Interface identification is specified in ERC-165. This function
    ///  uses less than 30,000 gas.
    /// @return `true` if the contract implements `interfaceID` and
    ///  `interfaceID` is not 0xffffffff, `false` otherwise
    function supportsInterface(bytes4 interfaceID) external view returns (bool);
}

abstract contract Test is ERC165 {
    function hello() public pure returns (bytes4 data){
        return type(super).interfaceID;
    }
}
// ----
// TypeError 4259: (592-597): Invalid type for argument in the function call. A contract type or an integer type is required, but type(contract super Test) provided.
