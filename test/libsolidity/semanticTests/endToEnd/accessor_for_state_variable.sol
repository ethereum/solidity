contract Lotto {
    uint public ticketPrice = 500;
}

// ====
// compileViaYul: also
// ----
// ticketPrice() -> 500
// ticketPrice():"" -> "500"
