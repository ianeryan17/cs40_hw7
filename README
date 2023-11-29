Ian Ryan (iryan01) and Caden Chang (cchang15)

Help Received: Sam (TA), Bill (TA), 

Implemented: all parts
Not Implemented: -

Fail cases mentioned in section 3.6 are handled with Hanson asserts, ending
the program when encountered.

Changes from original design:
- we implemented our original program as closely as possible to our design;
  however, when we ran the completed program against midmark.um, it took us
  40 minutes to finish, which made us realize that our initial design was not
  prioritizing efficiency/performance enough to be a useful emulator
- therefore, we made a series of changes to our program to accomplish this:
- changed segment representation from a table of sequences to a table of 
UArrays in order to limit overhead associated with Hanson structures
- used UArray_map to free all segments in table before freeing table
- changed map segment from segments.c functionality to only take in a length
and returns the selected segment ID
- unmapping segments completely removes key-value pair from the table, instead
of setting all words to 0
- our original loading program functionality manually copied each word to a new
segment, we replaced this with the UArray_copy function as this was much more
efficient
- in our design, we talked implementing our registers using a basic array. Our
first program actuallyy used a Hanson sequence to implement these registers,
which we realized was far too slow, and therefore we changed back to
using a basic array on the heap.
- in our design, we talked about potentially using function pointers or an
if/else ladder for execute_command(). After doing further research on more
efficient ways to do this, we learned about switch case statements, which we
believe are faster than either function pointers or if/else ladders. Therefore,
we implemented a switch case statement into execute_command().


Architecture:
um_driver.c:
- handles the main logic and functionality of UM 
- stores an instance of a registers array and implementation of segmented mem 
- responsible for reading each line of the program from the zero segment and 
calling functions that execute its logic
- never modifies the segments data structure, rather it calls functions from 
segments.c which is the interface for the segmented memory module

segments.c:
- responsible for modifying the data structures that represent  segmented mem
- mapped segments are stored in a table where key is segment ID and value is 
a sequence of 32-bit words
- mapped segments that are unmapped have their IDs stored in a sequence 

um.c:
- responsible for reading in the words of the program given as a um file
- passes a sequences of words to run() from um_driver.c to start the um

Our UM would complete 50 million instructions in 27 hours and roughly 47 
minutes. .We calculated this by taking the total time elapsed from hello.um
(10 instructions) and dividing 50,000,000 by that value.
10 / .02 = 500 instuctions per second
50,000,000 / 500 = 100,000 seconds = 27 hours, 47 minutes, and 40 seconds. 


UMTESTS:
halt.um
- tests the halt command by calling halt()

halt-verbose
- ensures that instructions after the halt instruction are not run by calling
 halt(), followed by other instructions

load_val.um
- tests single load_val instruction by loading a single val into a register

output.um
- tests single character output from a register

add.um
- tests add instruction in basic manner by calling it

print-six.um
- tests add instruction by adding two loaded values and then outputting them

map1.um
- tests single seg_map instructions

map2.um
- tests multiples seg_map instructions

map3.um
- tests mapping unmapped segment, as well as storing and loading into/from a
segment

input.um
- tests single input instruction by outputting a single input value

operations.um
- tests all operations together with the values 12 in r1 and 6 in r2

load_prog.um
- input program that outputs the text from the input stream, also tests
other opcode commands such as conditional_move and load_program
        - mirrors cat.um functionality, as we felt this was a good way to test
          multiple load_program and input calls

Hours Spent:
Analysing the assignment: 3 hours
Preparing your design: 10 hours
Solving the problems after your analysis: 20 hours