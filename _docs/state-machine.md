---
layout: docs
title: State Machine
permalink: /docs/state-machine/
---

Contracts often act as a state machine, which means
that they have certain stages in which they behave
differently or in which different functions can
be called. A function call often ends a stage
and transitions the contract into the next stage.

An example for this is a blind auction contract which
starts in the stage "accepting blinded bids", then
transitions to "revealing bids" which is ended by
"determine auction autcome".

Function modifiers can be used in this situation
to model the states and guard against
incorrect usage of the contract.

