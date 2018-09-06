pragma solidity ^0.4.11;
import "../Oracles/SignedMessageOracle.sol";


/// @title Signed message oracle factory contract - Allows to create signed message oracle contracts
/// @author Stefan George - <stefan@gnosis.pm>
contract SignedMessageOracleFactory {

    /*
     *  Events
     */
    event SignedMessageOracleCreation(address indexed creator, SignedMessageOracle signedMessageOracle, address oracle);

    /*
     *  Public functions
     */
    /// @dev Creates a new signed message oracle contract
    /// @param descriptionHash Hash identifying off chain event description
    /// @param v Signature parameter
    /// @param r Signature parameter
    /// @param s Signature parameter
    /// @return Oracle contract
    function createSignedMessageOracle(bytes32 descriptionHash, uint8 v, bytes32 r, bytes32 s)
        public
        returns (SignedMessageOracle signedMessageOracle)
    {
        signedMessageOracle = new SignedMessageOracle(descriptionHash, v, r, s);
        address oracle = ecrecover(descriptionHash, v, r, s);
        emit SignedMessageOracleCreation(msg.sender, signedMessageOracle, oracle);
    }
}
