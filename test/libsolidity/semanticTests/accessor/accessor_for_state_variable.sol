contract Lotto {
    uint256 public ticketPrice = 500;
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// ticketPrice() -> 500
