---
layout: docs
title: Fallback Function
permalink: /docs/fallback-function/
---

A contract can have exactly one unnamed function. This function cannot have
arguments and is executed on a call to the contract if none of the other
functions matches the given function identifier (or if no data was supplied at
all).

Furthermore, this function is executed whenever the contract receives plain
Ether (witout data).  In such a context, there is very little gas available to
the function call, so it is important to make fallback functions as cheap as
possible.



{% highlight javascript %}
contract Test {
  function() { x = 1; }
  uint x;
}

// This contract rejects any Ether sent to it. It is good
// practise to include such a function for every contract
// in order not to loose Ether.
contract Rejector {
  function() { throw; }
}

contract Caller {
  function callTest(address testAddress) {
    Test(testAddress).call(0xabcdef01); // hash does not exist
    // results in Test(testAddress).x becoming == 1.
    Rejector r = Rejector(0x123);
    r.send(2 ether);
    // results in r.balance == 0 
  }
}
{% endhighlight %}
