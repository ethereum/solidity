pragma solidity >=0.0;
import "Factory.sol";
import "MultiSigWallet.sol";


/// @title Multisignature wallet factory - Allows creation of multisig wallet.
/// @author Stefan George - <stefan.george@consensys.net>
contract MultiSigWalletFactory is Factory {

    /// @dev Allows verified creation of multisignature wallet.
    /// @param _owners List of initial owners.
    /// @param _required Number of required confirmations.
    /// @return wallet Returns wallet address.
    function create(address[] memory _owners, uint _required)
        public
        returns (address wallet)
    {
        wallet = address(new MultiSigWallet(_owners, _required));
        register(wallet);
    }
}
