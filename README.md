# UCSD_CSE_291G_Lab2

There is only one submission for Lab 2. This will contain:
    - Your explanation on how you set up a simple `flush+reload` covert channel.
    - Your explanation on how you set up a simple `spectre` attack.
    - Your answers to the 2 discussion questions.
    - If you are unable to set up a working spectre attack, please try to show your flush+reload code works with a simple victim function that loads a memory value non-speculatively.

## Discussion Question 1

In our example, the attacker tries to leak the values in the array `secret_array`. In a real world attack, attackers can use Spectre to leak data located in an arbitrary address in the victim’s space. Explain how an attacker can achieve such leakage.

## Discussion Question 2

Experiment with how often you train the branch predictor. What is the minimum number of times you need to train the branch to make the attack work?
