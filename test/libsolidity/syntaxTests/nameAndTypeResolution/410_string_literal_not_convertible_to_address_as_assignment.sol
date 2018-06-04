// A previous implementation claimed the string would be an address
contract AddrString {
    address public test = "0xCA35b7d915458EF540aDe6068dFe2F44E8fa733c";
}
// ----
// TypeError: (116-160): Type literal_string "0xCA35b7d915458EF540aDe6068dFe2F44E8fa733c" is not implicitly convertible to expected type address.
