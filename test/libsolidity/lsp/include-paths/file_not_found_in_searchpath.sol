// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

   import "test.sol";
// ^^^^^^^^^^^^^^^^^^ @IncludeLocation

contract SomeContract
{
}
// ----
// file_not_found_in_searchpath: @IncludeLocation 6275
