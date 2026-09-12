==== Source: https://example.com/token/Context.sol ====
contract Context {}
==== Source: https://example.com/token/Ownable.sol ====
import "./Context.sol";
contract Ownable is Context {}
==== Source: https://example.com/utils/Strings.sol ====
import "../token/Ownable.sol";
contract Strings is Ownable {}
