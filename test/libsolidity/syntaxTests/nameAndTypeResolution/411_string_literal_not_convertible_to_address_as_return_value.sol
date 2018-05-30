// A previous implementation claimed the string would be an address
contract AddrString {
    function f() public returns (address) {
        return "0xCA35b7d915458EF540aDe6068dFe2F44E8fa733c";
   }
}
// ----
// TypeError: (149-193): Return argument type literal_string "0xCA35b7d915458EF540aDe6068dFe2F44E8fa733c" is not implicitly convertible to expected type (type of first return variable) address.
