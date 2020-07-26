contract Lotto {
    uint256 public ticketPrice = 500;
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// ticketPrice() -> 500
