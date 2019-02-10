# TuringMachineSimulator
A non deterministic Turing Machine simulator written in C with focus on optimization.
The biggest improvement in performance is given from the reuse of the list nodes instead of creating new ones.
## Input file
The input file is the description of the machine and the list of 
- The first part starts with "tr", it contains the transactions, one for each line. 
 EX: 0 a b R 1 means it starts from 0 reading 'a' and goes to 1 writing 'b', 'R' means the tape goes to right (S=stop, L=left).
- Next, after the keyword "acc", comes the list of final states (one for each line).
- Next, after the keyword "max" there's the number of maximum transaction to test before saying the output.
- At the end, after the keyword "run" there's the list of strings to test.

All the computations starts from the state 0.
### Example
tr

0 a aR 0

0 b bR 0

0 a c R 1

0 b c R 2

1 a c L 3

2 b c L 3

3 c cL 3

3 a c R 4

3 b c R 5

4 c cR 4

4 a c L 3

5 c cR 5

5 b c L 3

3 _ _ R 6

6 c cR 6

6 _ _ S 7

acc

7

max

800

run

aababbabaa

aababbabaaaababbabaa

aababbabaaaababbabaab

aababbabaaaababbabaabbaababbabaaaababbabaa

aababbabbaaababbabaabbaababbabaaaababbabaa

Output:

1

1

0

U

0
