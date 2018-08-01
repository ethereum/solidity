pragma solidity ^0.4.11;

import "./safeMath.sol";

contract multiOwner is safeMath {

    mapping(address => bool) public owners;
    uint256 public ownerCount;

    mapping(bytes32 => address[]) public doDB;

    /*
        Constructor
    */
    constructor(address[] memory newOwners) public {
        for ( uint256 a=0 ; a<newOwners.length ; a++ ) {
            _addOwner(newOwners[a]);
        }
    }
    /*
        Externals
    */
    function insertOwner(address addr) external {
        if ( insertAndCheckDo(calcDoHash("insertOwner", keccak256(abi.encodePacked(addr)))) ) {
            _addOwner(addr);
        }
    }
    function dropOwner(address addr) external {
        if ( insertAndCheckDo(calcDoHash("dropOwner", keccak256(abi.encodePacked(addr)))) ) {
            _delOwner(addr);
        }
    }
    function cancelDo(bytes32 doHash) external {
        if ( insertAndCheckDo(calcDoHash("cancelDo", doHash)) ) {
            delete doDB[doHash];
        }
    }
    /*
        Constants
    */
    function ownersForChange() public view returns (uint256 owners) {
        return ownerCount * 75 / 100;
    }
    function calcDoHash(string memory job, bytes32 data) public pure returns (bytes32 hash) {
        return keccak256(abi.encodePacked(job, data));
    }
    function validDoHash(bytes32 doHash) public view returns (bool valid) {
        return doDB[doHash].length > 0;
    }
    /*
        Internals
    */
    function insertAndCheckDo(bytes32 doHash) internal returns (bool success) {
        require( owners[msg.sender] );
        if (doDB[doHash].length >= ownersForChange()) {
            delete doDB[doHash];
            return true;
        }
        for ( uint256 a=0 ; a<doDB[doHash].length ; a++ ) {
            require( doDB[doHash][a] != msg.sender );
        }
        if ( doDB[doHash].length+1 >= ownersForChange() ) {
            delete doDB[doHash];
            return true;
        } else {
            doDB[doHash].push(msg.sender);
            return false;
        }
    }
    /*
        Privates
    */
    function _addOwner(address addr) private {
        if ( owners[addr] ) { return; }
        owners[addr] = true;
        ownerCount = safeAdd(ownerCount, 1);
    }
    function _delOwner(address addr) private {
        if ( ! owners[addr] ) { return; }
        delete owners[addr];
        ownerCount = safeSub(ownerCount, 1);
    }
}
