type MyAddress is address;
type MyAddressPayable is address payable;
type MyInt is int;
type MyUInt is uint;
type MyInt128 is int128;
type MyUInt128 is uint128;
type MyFixed is fixed;
type MyUfixed is ufixed;
type MyFixedBytes32 is bytes32;
type MyFixedBytes1 is bytes1;
type MyBool is bool;
type MyCallback is function(uint256) returns (bool);
type MyPureFunction is function(uint256, uint256) pure returns (uint256);
/// test to see if having NatSpec causes issues
type redundantNatSpec is bytes2;
