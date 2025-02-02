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
