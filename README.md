Exercise 1.1. The dining philosophers problem was invented by E. W. Dijkstra, a
concurrency pioneer, to clarify the notions of deadlock and starvation freedom.
Imagine five philosophers who spend their lives just thinking and feasting. They
sit around a circular table with five chairs. The table has a big plate of rice. 
However, there are only five chopsticks (in the original formulation forks) available,
as shown in Fig. 1.5. Each philosopher thinks. When he gets hungry, he sits down
and picks up the two chopsticks that are closest to him. If a philosopher can pick
up both chopsticks, he can eat for a while. After a philosopher finishes eating, he
puts down the chopsticks and again starts to think.
1. Write a program to simulate the behavior of the philosophers, where each
philosopher is a thread and the chopsticks are shared objects. Notice that you
must prevent a situation where two philosophers hold the same chopstick at
the same time.
2. Amend your program so that it never reaches a state where philosophers are
deadlocked, that is, it is never the case that each philosopher holds one chopstick
and is stuck waiting for another to get the second chopstick.
3. Amend your program so that no philosopher ever starves.
4. Write a program to provide a starvation-free solution for any number of philosophers n.

Solution: [Dining Philosophers](https://github.com/volekkkkk/the-art-of-multiprocessing-programming/tree/master/dining_philosophers)


Exercise 1.2. For each of the following, state whether it is a safety or liveness property. 
Identify the bad or good thing of interest.

1. Patrons are served in the order they arrive. - [Safety]
    Bad thing: a newer patron is prioritized over older one.  
2. Anything that go wrong, will go wrong. - [Liveness]
    Good thing: something that go wrong, eventually go wrong.
3. No one wants to die. - [Safety]
    Bad thing: state of death.
4. Two things are certain: death and taxes. - [Liveness]
    Good thing: eventually you will die and pay taxes.
5. As soon as one is born, one starts dying. - [Liveness]
    Good thing: an one will eventually die.
6. If an interrupt occurs, then a message is printed within one second. - [Safety]
    Bad thing: a message is printed in descrete time.
7. If an interrupt occurs, then a message is printed. - [Liveness]
    Good thing: a message is eventually printed.
8. I will finish what Darth Vader has started. - [Liveness]
    Good thing: the work of Darth Vader will progress.
9. The cost of living never decreases. - [Safety]
    Good thing: the cost of living decreses at no point in time.
10. You can always tell a Harvard man. - [Liveness]
    Good thing: you will eventually recognize a Harvard man.


Exercise 1.3. In the producer-consumer fable, we assumed that Bob can see whether the can on Alice's windowsill is up or down.
Design a producer-consumer protocol using cans and strings that works even if Bob cannot see the state of Alice's can (this
is how real-world interrupt bits work).

Bob and Alice have two separate cans connected by a string.
At first, there is no food in the yard. Alice's can is hanging from the windowsill, and Bob's is standing on the windowsill.
Bob puts the food in the yard and throws his can off the windowsill, so that the can on the other side is pulled up to the windowsill and stands on it.
When there is no food in the yard, Alice drops the can from the windowsill. The string stretches, and Bob's can 
is pulled up onto the windowsill. In this way, Bob relies on the state of his can and does not have to see Alice's can. 


Exercise 1.4. You are one of P recently arrested prisoners. The warden, a deranged computer scientist, makes the following announcement:
> You may meet together today and plan a strategy, but after today you will be in
> isolated cells and have no communication with one another.
> I have set up a “switch room” which contains a light switch, which is either on or
> off. The switch is not connected to anything.
> Every now and then, I will select one prisoner at random to enter the “switch
> room.” This prisoner may throw the switch (from on to off, or vice-versa), or may
> leave the switch unchanged. Nobody else will ever enter this room.
> Each prisoner will visit the switch room arbitrarily often. More precisely, for any
> N, eventually each of you will visit the switch room at least N times.
> At any time, any of you may declare: “we have all visited the switch room at least
> once.” If the claim is correct, I will set you free. If the claim is incorrect, I will feed
> all of you to the crocodiles. Choose wisely!
Hint: not all prisoners need to do the same thing.

- Devise a winning strategy when you know that the initial state of the switch
is off.

Solution: One prisoner is a counter. He turns off the light in the room if it is on and counts
each time he does so. When he turns off the light (P-1) times, he declares that all the prisoners
have been in the room. Each of the other prisoners must turn on the light when he enters the room
and the switch is off (and he does this only once: if a prisoner turns on the light and then re-enters
the room, no new actions occur).

- Devise a winning strategy when you do not know whether the initial state of
the switch is on or off.

Solution: the same as above, but each prisoner turns on light twice, condition for exit: 2(P-1) - 1.
