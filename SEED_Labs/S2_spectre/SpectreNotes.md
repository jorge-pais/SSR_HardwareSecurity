# Systems and Network Security - Project

## Spectre State of Art

Spectre is a hardware vulnerability that was first discovered in 2017. It affects a wide range of modern processors, including those made by Intel, AMD, and ARM. The vulnerability allows an attacker to exploit the speculative execution feature of these processors to gain unauthorized access to sensitive data, such as passwords and encryption keys.

Since the discovery of Spectre, there have been ongoing efforts to mitigate the vulnerability. The state of the art with respect to Spectre mitigation involves a combination of software and hardware solutions.

Software mitigations include techniques such as recompiling software with "retpoline" code, which prevents the processor from speculatively executing instructions that could be exploited by an attacker. Operating system updates have also been released that include kernel-level mitigations, such as reducing the amount of data that can be accessed through the kernel interface.

Hardware mitigations involve changes to the processor architecture itself. Newer processors, such as those released by Intel and AMD in 2018 and later, include hardware mitigations that make it more difficult for attackers to exploit Spectre vulnerabilities. These hardware mitigations include changes to the way branch prediction works, as well as new instructions that allow software to better control speculative execution.

Overall, the state of the art with respect to Spectre mitigation is constantly evolving as researchers and manufacturers continue to develop new techniques for detecting and mitigating the vulnerability. However, it is important to note that even with these mitigations in place, Spectre is still considered a significant threat to computer security, and users should take steps to protect themselves, such as keeping their software and operating systems up to date, and being cautious about downloading and running untrusted software.

---

## Seed Labs TASKS

### Task 1 - Cache Access Times vs RAM

This task was tested in three different machines, two vulnerable Intel systems with a i7-8550U (Whiskey Lake) and with a i7-4600U, and a hardware patched AMD system running a Ryzen 5 2600 (Zen+).

Running the `CacheTime.c` program, it is clear that with both systems there is a clear separation of the memory access times. Although, while in the vulnerable Intel system the access times for `array[3*4096]` and `array[7*4096]` were always lower than those of the rest of the array, the patched Ryzen system showed some cases where there were times lower than these. This could be due to the larger cache amount of the Ryzen architecture.

The results can be found in the `results.ods` file. From these it was possible to conclude that the access time threshold could be set around the __200__ Clock Cycle mark. For the specific Intel i7-4600U it was chosen to be __260__, which is greater than the average value so as to encompase deviatons from the mean value.

### Task 2 - Flush and Reload
Supposing we have a victim function that acesses some secret value from a array, we can use the flush and reload technique to read this secret value.
The flush and reload is done as follows:

- Flush all the array contents from cache

- Call the victim function, caching the secret array element

- Reload the entire array, measuring the time that it takes to access each of the elements

Using the `FlushReload.c` program, one should be able to set a access time threshold (discovered in the previous task) and be able to find what was accessed by the victim function. 
Running the program it is possible to see that the "secret" array position is found sometimes. The problem with this is that the access times are highly variable, and sometimes even the cached array values take longer to return, and sometimes the non-cached array values are also detected has having been cached. We can conclude that this technique, is highly dependent on the threshold value that is set, and this means that using the CPU's cache as an attack side-channel will require many attempts.

For the Intel i7-4600U, the program mentioned was run, achiving success (only cache hit on the secret) 92 out of the 100 times.

### Task 3 - Out-of-Order Execution and Branch Prediction
Now taking advantage of Out-of-order execution, we'll try to access a secret value by first training the out-of-order execution unit to always take a branch (in this case, the if statement inside of the victim function), then we'll try to call the same function with a value that should not be loaded, finally we'll reload the vector and check the access times. As with the `FlushReload.c` program, this for now has to be run multiple times due to the noisy cache channel.

If we now comment the `_mm_clflush(&size);` line inside of `main()`, we can see that the attack no longer works.This is because now the `size` variable is no longer being cleared from cache, and so the out-of-order execution unit is able to determine the outcome of the if-statement without speculation, as such the secret is never loaded from memory to cache.

If now we change the `victim(i);` line to `victim(i+20);` inside of the training for-loop, then the out-of-order execution unit will be trained to not predict a true condition, since all the values that are run within the training phase will lead to false conditions within the if-statement (as long as size is set to the default value 10). As so, this experiment will never return the secret value

### Task 4 - The ACTUAL Spectre Attack
Running the attack program `SpectreAttack.c` it is possible to see that the attack is actually working and is able to get the first letter of the secret string. During the reload stage of the side channel, another value that was stored in cache was that of `array[0*4096 + 1024]`, this is the actual Bug that makes Spectre work. 

So basically the effect of out-of-order execution made it so that two values were written to the cache, firstly the "wrong one" a.k.a "the secret" and when the value of the branch condition was determined, the correct value was written to the other position.

_**TODO:** Show that this doesn't work on the patched system_

_**Strange Behaviour:** Running the attack program from my actual system (which according to to spectre-meltdown-checker in not vulnerable to either attack) actually gives out the correct result. This could be due to the fact that the attack is not breaking any inter-process barriers._

### Task 5 - The CLEANER Spectre Attack

Running the attack in it's current form makes it as such the maximum number of hits is that of the (0\*4096+1024) array position. When running the code, every time the condition is taken, in the end the value returned by `RestrictedAccess` will be 0, and so the array[0\*4096+1024] is allways cached.

A possible solution is to ignore the score given to the corresponding position 0 during the evaluation of maximum hits.

We tested the program with and whitout the printf call and we confirmed that this line of code is essential for the attack to work properly.
We have tested with other system calls, like usleep, but we weren't able to achieve an equivalent behaviour.
Also, we tested with a repeting patern of incrementing and decrementing a value of a variable, with varying success. We noticed that the success in retrieving the secret message, increased with the number of repetitions.
We can conclude, that this line of code has to stall the program execution, without stopping it.

Lastly, we increased the time between `SpectreAttack` and `reloadSideChannelImproved`, with usleep funtion. Increasing its time, guarantes greater success in the retrieval of the secret message. We think this is due to the fact that during the score atribution (`reloadSideChannelImproved`) the wrong value has time to be loaded to the cache.




