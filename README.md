# UCSD_CSE_291G_Lab2

There is only one submission for Lab 2. This will contain:

- Explanation on how you set up a simple `flush+reload` covert channel.
- Explanation on how you set up a simple `spectre` attack.
- Answers to the 2 discussion questions.
- If you are unable to set up a working `spectre` attack, please try to show your `flush+reload` code works with a simple victim function that loads a memory value non-speculatively.

## Flush + Reload

First, a large block of memory (`secret_array`) is allocated using `mmap` with huge pages to create a buffer. Constants that referred to the cache structure were used to align data with the L1 cache, helping target specific cache lines for each element in `secret_array`.

- In the **flush phase**, each cache line in `secret_array` is flushed from the cache using the `clflush` instruction, ensuring that no entries reside in the L1 cache. This step establishes a baseline where accesses to `secret_array` should result in a cache miss.
- Next, in the **load phase**, a specific element (`secret_array[47]`) is accessed, bringing the corresponding cache line into the L1 cache. 
- Finally, in the **reload phase**, the code measures access times for each line in `secret_array`. 

By timing these accesses with `measure_one_block_access_time`, it identifies which lines are cached. If an access time is below a predefined threshold (`L1_THRESHOLD`), it indicates a cache hit, showing that the line was loaded into the cache.

## Spectre

This code demonstrates a basic Spectre attack setup by using speculative execution and cache timing to infer secret data from `secret_array`. Like a `flush+reload` attack, it leverages cache timing to detect whether specific data has been loaded into the cache. However, Spectre exploits speculative execution by manipulating branch prediction, allowing it to access data outside normal security bounds.

First, a `huge_page` is allocated using `mmap`, containing both `secret_array` and `shared_mem`.

- `secret_array` is initialized with integer values
- `shared_mem` is allocated at an offset within the same huge page to keep memory contiguous.

The key component, `victim_function`, simulates speculative execution by first performing a bounds check on `offset`. If the check passes, it retrieves a secret value from `secret_array`, then accesses `shared_mem` at an index derived from the secret. This creates a side effect in the cache based on the secret value.

The attack has five main phases:

- In the **mistraining phase**, `victim_function` is called multiple times with in-bounds `offset` values to train the branch predictor to expect the bounds check to pass.
- In the **flush phase**, `clflush` is used to evict both `secret_array` and `shared_mem` from the cache.
- The **load phase** triggers speculative execution by calling `victim_function` with an out-of-bounds `offset`, causing the branch predictor to speculatively execute the memory access. During this speculative execution, a specific line in `shared_mem` is cached based on the secret value from `secret_array`.
- Finally, in the **reload phase**, cache access times to `shared_mem` are measured.

By timing each access, the code identifies which parts of `shared_mem` were cached, indirectly revealing the secret. This approach leverages both speculative execution and cache timing, using Spectre to bypass normal access bounds and retrieve protected data.

## Discussion Question 1

In our example, the attacker tries to leak the values in the array `secret_array`. In a real world attack, attackers can use Spectre to leak data located in an arbitrary address in the victim’s space. Explain how an attacker can achieve such leakage.

In a real-world Spectre attack, an attacker exploits speculative execution to leak data from arbitrary addresses in a victim's memory. Although speculative results are discarded if mispredicted, changes to the cache persist and can be detected through timing side-channel attacks.

To leak data, an attacker identifies code in the victim’s program with conditional memory access, such as `if (offset < limit) { secret_data = array[offset]; }`, and (mis)trains the branch predictor to expect the condition to be true. Then, by providing an out-of-bounds value for `offset`, the attacker causes the branch predictor to speculatively execute the body of the conditional, getting access to restricted memory as a side channel.

The attacker is setting up a cache side channel by accessing a secondary array like `shared_mem[mem_index]`, where `mem_index` depends on the secret data. This speculative access leaves a cache trace that the attacker can observe by measuring access times to `shared_mem`. Repeating this process allows reconstruction of sensitive data from the victim’s memory.

- Faster access to specific indices reveals which parts of the data were accessed, enabling the attacker to infer the value of `secret_array[x]`.
- Slower access reveals that part of memory has not been accessed by the victim.

## Discussion Question 2

Experiment with how often you train the branch predictor. What is the minimum number of times you need to train the branch to make the attack work?

After experimenting, I was able to get nearly 100% accuracy around 10 iterations of training. However, it was already fairly accurate with less, but saturated around 10 iterations.
