interface ERC165 {
    /// @notice Query if a contract implements an interface
    /// @param interfaceID The interface identifier, as specified in ERC-165
    /// @dev Interface identification is specified in ERC-165. This function
    ///  uses less than 30,000 gas.
    /// @return `true` if the contract implements `interfaceID` and
    ///  `interfaceID` is not 0xffffffff, `false` otherwise
    function supportsInterface(bytes4 interfaceID) external view returns (bool);
}

interface Simpson {
    function is2D() external returns (bool);
    function skinColor() external returns (string memory);
}

contract Homer is ERC165, Simpson {
    function supportsInterface(bytes4 interfaceID) external view override returns (bool) {
        return
            interfaceID == type(ERC165).interfaceId ||
            interfaceID == type(Simpson).interfaceId;
    }

    function is2D() external override returns (bool) {
        return true;
    }

    function skinColor() external override returns (string memory) {
        return "yellow";
    }
}
// ----
// supportsInterface(bytes4): left(0x01ffc9a0) -> false
// supportsInterface(bytes4): left(0x01ffc9a7) -> true
// supportsInterface(bytes4): left(0x73b6b492) -> true
// supportsInterface(bytes4): left(0x70b6b492) -> false
