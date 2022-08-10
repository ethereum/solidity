// ┏━━━┓━┏┓━┏┓━━┏━━━┓━━┏━━━┓━━━━┏━━━┓━━━━━━━━━━━━━━━━━━━┏┓━━━━━┏━━━┓━━━━━━━━━┏┓━━━━━━━━━━━━━━┏┓━
// ┃┏━━┛┏┛┗┓┃┃━━┃┏━┓┃━━┃┏━┓┃━━━━┗┓┏┓┃━━━━━━━━━━━━━━━━━━┏┛┗┓━━━━┃┏━┓┃━━━━━━━━┏┛┗┓━━━━━━━━━━━━┏┛┗┓
// ┃┗━━┓┗┓┏┛┃┗━┓┗┛┏┛┃━━┃┃━┃┃━━━━━┃┃┃┃┏━━┓┏━━┓┏━━┓┏━━┓┏┓┗┓┏┛━━━━┃┃━┗┛┏━━┓┏━┓━┗┓┏┛┏━┓┏━━┓━┏━━┓┗┓┏┛
// ┃┏━━┛━┃┃━┃┏┓┃┏━┛┏┛━━┃┃━┃┃━━━━━┃┃┃┃┃┏┓┃┃┏┓┃┃┏┓┃┃━━┫┣┫━┃┃━━━━━┃┃━┏┓┃┏┓┃┃┏┓┓━┃┃━┃┏┛┗━┓┃━┃┏━┛━┃┃━
// ┃┗━━┓━┃┗┓┃┃┃┃┃┃┗━┓┏┓┃┗━┛┃━━━━┏┛┗┛┃┃┃━┫┃┗┛┃┃┗┛┃┣━━┃┃┃━┃┗┓━━━━┃┗━┛┃┃┗┛┃┃┃┃┃━┃┗┓┃┃━┃┗┛┗┓┃┗━┓━┃┗┓
// ┗━━━┛━┗━┛┗┛┗┛┗━━━┛┗┛┗━━━┛━━━━┗━━━┛┗━━┛┃┏━┛┗━━┛┗━━┛┗┛━┗━┛━━━━┗━━━┛┗━━┛┗┛┗┛━┗━┛┗┛━┗━━━┛┗━━┛━┗━┛
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┃┃━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┗┛━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

// SPDX-License-Identifier: CC0-1.0

// This interface is designed to be compatible with the Vyper version.
/// @notice This is the Ethereum 2.0 deposit contract interface.
/// For more information see the Phase 0 specification under https://github.com/ethereum/eth2.0-specs
interface IDepositContract {
    /// @notice A processed deposit event.
    event DepositEvent(
        bytes pubkey,
        bytes withdrawal_credentials,
        bytes amount,
        bytes signature,
        bytes index
    );

    /// @notice Submit a Phase 0 DepositData object.
    /// @param pubkey A BLS12-381 public key.
    /// @param withdrawal_credentials Commitment to a public key for withdrawals.
    /// @param signature A BLS12-381 signature.
    /// @param deposit_data_root The SHA-256 hash of the SSZ-encoded DepositData object.
    /// Used as a protection against malformed input.
    function deposit(
        bytes calldata pubkey,
        bytes calldata withdrawal_credentials,
        bytes calldata signature,
        bytes32 deposit_data_root
    ) external payable;

    /// @notice Query the current deposit root hash.
    /// @return The deposit root hash.
    function get_deposit_root() external view returns (bytes32);

    /// @notice Query the current deposit count.
    /// @return The deposit count encoded as a little endian 64-bit number.
    function get_deposit_count() external view returns (bytes memory);
}

// Based on official specification in https://eips.ethereum.org/EIPS/eip-165
interface ERC165 {
    /// @notice Query if a contract implements an interface
    /// @param interfaceId The interface identifier, as specified in ERC-165
    /// @dev Interface identification is specified in ERC-165. This function
    ///  uses less than 30,000 gas.
    /// @return `true` if the contract implements `interfaceId` and
    ///  `interfaceId` is not 0xffffffff, `false` otherwise
    function supportsInterface(bytes4 interfaceId) external pure returns (bool);
}

// This is a rewrite of the Vyper Eth2.0 deposit contract in Solidity.
// It tries to stay as close as possible to the original source code.
/// @notice This is the Ethereum 2.0 deposit contract interface.
/// For more information see the Phase 0 specification under https://github.com/ethereum/eth2.0-specs
contract DepositContract is IDepositContract, ERC165 {
    uint constant DEPOSIT_CONTRACT_TREE_DEPTH = 32;
    // NOTE: this also ensures `deposit_count` will fit into 64-bits
    uint constant MAX_DEPOSIT_COUNT = 2**DEPOSIT_CONTRACT_TREE_DEPTH - 1;

    bytes32[DEPOSIT_CONTRACT_TREE_DEPTH] branch;
    uint256 deposit_count;

    bytes32[DEPOSIT_CONTRACT_TREE_DEPTH] zero_hashes;

    constructor() public {
        // Compute hashes in empty sparse Merkle tree
        for (uint height = 0; height < DEPOSIT_CONTRACT_TREE_DEPTH - 1; height++)
            zero_hashes[height + 1] = sha256(abi.encodePacked(zero_hashes[height], zero_hashes[height]));
    }

    function get_deposit_root() override external view returns (bytes32) {
        bytes32 node;
        uint size = deposit_count;
        for (uint height = 0; height < DEPOSIT_CONTRACT_TREE_DEPTH; height++) {
            if ((size & 1) == 1)
                node = sha256(abi.encodePacked(branch[height], node));
            else
                node = sha256(abi.encodePacked(node, zero_hashes[height]));
            size /= 2;
        }
        return sha256(abi.encodePacked(
            node,
            to_little_endian_64(uint64(deposit_count)),
            bytes24(0)
        ));
    }

    function get_deposit_count() override external view returns (bytes memory) {
        return to_little_endian_64(uint64(deposit_count));
    }

    function deposit(
        bytes calldata pubkey,
        bytes calldata withdrawal_credentials,
        bytes calldata signature,
        bytes32 deposit_data_root
    ) override external payable {
        // Extended ABI length checks since dynamic types are used.
        require(pubkey.length == 48, "DepositContract: invalid pubkey length");
        require(withdrawal_credentials.length == 32, "DepositContract: invalid withdrawal_credentials length");
        require(signature.length == 96, "DepositContract: invalid signature length");

        // Check deposit amount
        require(msg.value >= 1 ether, "DepositContract: deposit value too low");
        require(msg.value % 1 gwei == 0, "DepositContract: deposit value not multiple of gwei");
        uint deposit_amount = msg.value / 1 gwei;
        require(deposit_amount <= type(uint64).max, "DepositContract: deposit value too high");

        // Emit `DepositEvent` log
        bytes memory amount = to_little_endian_64(uint64(deposit_amount));
        emit DepositEvent(
            pubkey,
            withdrawal_credentials,
            amount,
            signature,
            to_little_endian_64(uint64(deposit_count))
        );

        // Compute deposit data root (`DepositData` hash tree root)
        bytes32 pubkey_root = sha256(abi.encodePacked(pubkey, bytes16(0)));
        bytes32 signature_root = sha256(abi.encodePacked(
            sha256(abi.encodePacked(signature[:64])),
            sha256(abi.encodePacked(signature[64:], bytes32(0)))
        ));
        bytes32 node = sha256(abi.encodePacked(
            sha256(abi.encodePacked(pubkey_root, withdrawal_credentials)),
            sha256(abi.encodePacked(amount, bytes24(0), signature_root))
        ));

        // Verify computed and expected deposit data roots match
        require(node == deposit_data_root, "DepositContract: reconstructed DepositData does not match supplied deposit_data_root");

        // Avoid overflowing the Merkle tree (and prevent edge case in computing `branch`)
        require(deposit_count < MAX_DEPOSIT_COUNT, "DepositContract: merkle tree full");

        // Add deposit data root to Merkle tree (update a single `branch` node)
        deposit_count += 1;
        uint size = deposit_count;
        for (uint height = 0; height < DEPOSIT_CONTRACT_TREE_DEPTH; height++) {
            if ((size & 1) == 1) {
                branch[height] = node;
                return;
            }
            node = sha256(abi.encodePacked(branch[height], node));
            size /= 2;
        }
        // As the loop should always end prematurely with the `return` statement,
        // this code should be unreachable. We assert `false` just to be safe.
        assert(false);
    }

    function supportsInterface(bytes4 interfaceId) override external pure returns (bool) {
        return interfaceId == type(ERC165).interfaceId || interfaceId == type(IDepositContract).interfaceId;
    }

    function to_little_endian_64(uint64 value) internal pure returns (bytes memory ret) {
        ret = new bytes(8);
        bytes8 bytesValue = bytes8(value);
        // Byteswapping during copying to bytes.
        ret[0] = bytesValue[7];
        ret[1] = bytesValue[6];
        ret[2] = bytesValue[5];
        ret[3] = bytesValue[4];
        ret[4] = bytesValue[3];
        ret[5] = bytesValue[2];
        ret[6] = bytesValue[1];
        ret[7] = bytesValue[0];
    }
}
// ----
// constructor()
// gas irOptimized: 1413443
// gas legacy: 2402189
// gas legacyOptimized: 1758087
// supportsInterface(bytes4): 0x0 -> 0
// supportsInterface(bytes4): 0xffffffff00000000000000000000000000000000000000000000000000000000 -> false # defined to be false by ERC-165 #
// supportsInterface(bytes4): 0x01ffc9a700000000000000000000000000000000000000000000000000000000 -> true # ERC-165 id #
// supportsInterface(bytes4): 0x8564090700000000000000000000000000000000000000000000000000000000 -> true # the deposit interface id #
// get_deposit_root() -> 0xd70a234731285c6804c2a4f56711ddb8c82c99740f207854891028af34e27e5e
// gas irOptimized: 112962
// gas legacy: 146048
// gas legacyOptimized: 119711
// get_deposit_count() -> 0x20, 8, 0 # TODO: check balance and logs after each deposit #
// deposit(bytes,bytes,bytes,bytes32), 32 ether: 0 -> FAILURE # Empty input #
// get_deposit_root() -> 0xd70a234731285c6804c2a4f56711ddb8c82c99740f207854891028af34e27e5e
// gas irOptimized: 112962
// gas legacy: 146048
// gas legacyOptimized: 119711
// get_deposit_count() -> 0x20, 8, 0
// deposit(bytes,bytes,bytes,bytes32), 1 ether: 0x80, 0xe0, 0x120, 0xaa4a8d0b7d9077248630f1a4701ae9764e42271d7f22b7838778411857fd349e, 0x30, 0x933ad9491b62059dd065b560d256d8957a8c402cc6e8d8ee7290ae11e8f73292, 0x67a8811c397529dac52ae1342ba58c9500000000000000000000000000000000, 0x20, 0x00f50428677c60f997aadeab24aabf7fceaef491c96a52b463ae91f95611cf71, 0x60, 0xa29d01cc8c6296a8150e515b5995390ef841dc18948aa3e79be6d7c1851b4cbb, 0x5d6ff49fa70b9c782399506a22a85193151b9b691245cebafd2063012443c132, 0x4b6c36debaedefb7b2d71b0503ffdc00150aaffd42e63358238ec888901738b8 -> # txhash: 0x7085c586686d666e8bb6e9477a0f0b09565b2060a11f1c4209d3a52295033832 #
// ~ emit DepositEvent(bytes,bytes,bytes,bytes,bytes): 0xa0, 0x0100, 0x0140, 0x0180, 0x0200, 0x30, 0x933ad9491b62059dd065b560d256d8957a8c402cc6e8d8ee7290ae11e8f73292, 0x67a8811c397529dac52ae1342ba58c9500000000000000000000000000000000, 0x20, 0xf50428677c60f997aadeab24aabf7fceaef491c96a52b463ae91f95611cf71, 0x08, 0xca9a3b00000000000000000000000000000000000000000000000000000000, 0x60, 0xa29d01cc8c6296a8150e515b5995390ef841dc18948aa3e79be6d7c1851b4cbb, 0x5d6ff49fa70b9c782399506a22a85193151b9b691245cebafd2063012443c132, 0x4b6c36debaedefb7b2d71b0503ffdc00150aaffd42e63358238ec888901738b8, 0x08, 0x00
// get_deposit_root() -> 0x2089653123d9c721215120b6db6738ba273bbc5228ac093b1f983badcdc8a438
// gas irOptimized: 112947
// gas legacy: 146058
// gas legacyOptimized: 119724
// get_deposit_count() -> 0x20, 8, 0x0100000000000000000000000000000000000000000000000000000000000000
// deposit(bytes,bytes,bytes,bytes32), 32 ether: 0x80, 0xe0, 0x120, 0xdbd986dc85ceb382708cf90a3500f500f0a393c5ece76963ac3ed72eccd2c301, 0x30, 0xb2ce0f79f90e7b3a113ca5783c65756f96c4b4673c2b5c1eb4efc22280259441, 0x06d601211e8866dc5b50dc48a244dd7c00000000000000000000000000000000, 0x20, 0x00344b6c73f71b11c56aba0d01b7d8ad83559f209d0a4101a515f6ad54c89771, 0x60, 0x945caaf82d18e78c033927d51f452ebcd76524497b91d7a11219cb3db6a1d369, 0x7595fc095ce489e46b2ef129591f2f6d079be4faaf345a02c5eb133c072e7c56, 0x0c6c3617eee66b4b878165c502357d49485326bc6b31bc96873f308c8f19c09d -> # txhash: 0x404d8e109822ce448e68f45216c12cb051b784d068fbe98317ab8e50c58304ac #
// ~ emit DepositEvent(bytes,bytes,bytes,bytes,bytes): 0xa0, 0x0100, 0x0140, 0x0180, 0x0200, 0x30, 0xb2ce0f79f90e7b3a113ca5783c65756f96c4b4673c2b5c1eb4efc22280259441, 0x06d601211e8866dc5b50dc48a244dd7c00000000000000000000000000000000, 0x20, 0x344b6c73f71b11c56aba0d01b7d8ad83559f209d0a4101a515f6ad54c89771, 0x08, 0x40597307000000000000000000000000000000000000000000000000000000, 0x60, 0x945caaf82d18e78c033927d51f452ebcd76524497b91d7a11219cb3db6a1d369, 0x7595fc095ce489e46b2ef129591f2f6d079be4faaf345a02c5eb133c072e7c56, 0x0c6c3617eee66b4b878165c502357d49485326bc6b31bc96873f308c8f19c09d, 0x08, 0x0100000000000000000000000000000000000000000000000000000000000000
// get_deposit_root() -> 0x40255975859377d912c53aa853245ebd939bdd2b33a28e084babdcc1ed8238ee
// gas irOptimized: 112947
// gas legacy: 146058
// gas legacyOptimized: 119724
// get_deposit_count() -> 0x20, 8, 0x0200000000000000000000000000000000000000000000000000000000000000
