interface ERC165 {
    /// @notice Query if a contract implements an interface
    /// @param interfaceID The interface identifier, as specified in ERC-165
    /// @dev Interface identification is specified in ERC-165. This function
    ///  uses less than 30,000 gas.
    /// @return `true` if the contract implements `interfaceID` and
    ///  `interfaceID` is not 0xffffffff, `false` otherwise
    function supportsInterface(bytes4 interfaceID) external view returns (bool);
}

contract ERC165MappingImplementation is ERC165 {
    /// @dev You must not set element 0xffffffff to true
    mapping(bytes4 => bool) internal supportedInterfaces;

    constructor() internal {
        supportedInterfaces[this.supportsInterface.selector] = true;
    }

    function supportsInterface(bytes4 interfaceID) external view override returns (bool) {
        return supportedInterfaces[interfaceID];
    }
}

interface Simpson {
    function is2D() external returns (bool);
    function skinColor() external returns (string memory);
}

contract Lisa is ERC165MappingImplementation, Simpson {
    constructor() public {
        supportedInterfaces[type(Simpson).interfaceId] = true;
    }

    function is2D() external override returns (bool) {
        return true;
    }

    function skinColor() external override returns (string memory) {
        return "yellow";
    }
}

// ====
// compileViaYul: also
// ----
// supportsInterface(bytes4): left(0x01ffc9a0) -> false
// supportsInterface(bytes4): left(0x01ffc9a7) -> true
// supportsInterface(bytes4): left(0x73b6b492) -> true
// supportsInterface(bytes4): left(0x70b6b492) -> false
