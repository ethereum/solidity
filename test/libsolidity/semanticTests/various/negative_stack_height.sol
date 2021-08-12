contract C {
    mapping(uint256 => Invoice) public invoices;
    struct Invoice {
        uint256 AID;
        bool Aboola;
        bool Aboolc;
        bool exists;
    }

    function nredit(uint256 startindex)
        public
        pure
        returns (
            uint256[500] memory CIDs,
            uint256[500] memory dates,
            uint256[500] memory RIDs,
            bool[500] memory Cboolas,
            uint256[500] memory amounts
        )
    {}

    function return500InvoicesByDates(
        uint256 begindate,
        uint256 enddate,
        uint256 startindex
    )
        public
        view
        returns (
            uint256[500] memory AIDs,
            bool[500] memory Aboolas,
            uint256[500] memory dates,
            bytes32[3][500] memory Abytesas,
            bytes32[3][500] memory bytesbs,
            bytes32[2][500] memory bytescs,
            uint256[500] memory amounts,
            bool[500] memory Aboolbs,
            bool[500] memory Aboolcs
        )
    {}

    function return500PaymentsByDates(
        uint256 begindate,
        uint256 enddate,
        uint256 startindex
    )
        public
        view
        returns (
            uint256[500] memory BIDs,
            uint256[500] memory dates,
            uint256[500] memory RIDs,
            bool[500] memory Bboolas,
            bytes32[3][500] memory bytesbs,
            bytes32[2][500] memory bytescs,
            uint256[500] memory amounts,
            bool[500] memory Bboolbs
        )
    {}
}

// via yul disabled because of stack issues.

// ====
// compileViaYul: also
// ----
// constructor() ->
