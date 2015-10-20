---
layout: docs
title: State Machine
permalink: /docs/state-machine/
---

Contracts often act as a state machine, which means
that they have certain **stages** in which they behave
differently or in which different functions can
be called. A function call often ends a stage
and transitions the contract into the next stage
(especially if the contract models **interaction**).
It is also common that some stages are automatically
reached at a certain point in **time**.

An example for this is a blind auction contract which
starts in the stage "accepting blinded bids", then
transitions to "revealing bids" which is ended by
"determine auction autcome".

Function modifiers can be used in this situation
to model the states and guard against
incorrect usage of the contract.

### Example

In the following example,
the modifier `atStage` ensures that the function can
only be called at a certain stage.

Automatic timed transitions
are handled by the modifier `timeTransitions`, which
should be used for all functions.

<div class="note info">
<h5>Modifier Order Matters</h5>
<p>If atStage is combined
with timedTransitions, make sure that you mention
it after the latter, so that the new stage is
taken into account.</p>
</div>

Finally, the modifier `transitionNext` can be used
to automatically go to the next stage when the
function finishes.

<div class="note info">
<h5>Modifier May be Skipped</h5>
<p>Since modifiers are applied by simply replacing
code and not by using a function call,
the code in the transitionNext modifier
can be skipped if the function itself uses
return. If you want to do that, make sure
to call nextStage manually from those functions.</p>
</div>

{% include open_link gist="0a221eaceb6d708bf271" %}
{% highlight javascript %}
contract StateMachine {
    enum Stages {
        AcceptingBlindedBids,
        RevealBids,
        AnotherStage,
        AreWeDoneYet,
        Finished
    }
    // This is the current stage.
    Stages public stage = Stages.AcceptingBlindedBids;

    uint public creationTime = now;

    modifier atStage(Stages _stage) {
        if (stage != _stage) throw;
        _
    }
    function nextStage() internal {
        stage = Stages(uint(stage) + 1);
    }
    // Perform timed transitions. Be sure to mention
    // this modifier first, otherwise the guards
    // will not take the new stage into account.
    modifier timedTransitions() {
        if (stage == Stages.AcceptingBlindedBids &&
                    now >= creationTime + 10 days)
            nextStage();
        if (stage == Stages.RevealBids &&
                now >= creationTime + 12 days)
            nextStage();
        // The other stages transition by transaction
    }
    
    // Order of the modifiers matters here!
    function bid()
        timedTransitions
        atStage(Stages.AcceptingBlindedBids)
    {
        // We will not implement that here
    }
    function reveal()
        timedTransitions
        atStage(Stages.RevealBids)
    {
    }

    // This modifier goes to the next stage
    // after the function is done.
    // If you use `return` in the function,
    // `nextStage` will not be called
    // automatically.
    modifier transitionNext()
    {
        _
        nextStage();
    }
    function g()
        timedTransitions
        atStage(Stages.AnotherStage)
        transitionNext
    {
        // If you want to use `return` here,
        // you have to call `nextStage()` manually.
    }
    function h()
        timedTransitions
        atStage(Stages.AreWeDoneYet)
        transitionNext
    {
    }
    function i()
        timedTransitions
        atStage(Stages.Finished)
    {
    }
}
{% endhighlight %}
